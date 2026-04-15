#include <WiFi.h>

// WiFi Settings
const char* ssid = "your_wifi_ssid";        // user to change
const char* password = "your_wifi_password";
const int tcp_port = 8080;

WiFiServer server(tcp_port);
WiFiClient client;

// Pin Definitions for 3 Channels + Button
#define PIN_JAW 0
#define PIN_CHIN 1
#define PIN_EAR 2
#define PIN_BTN 5  
#define LED_PIN 8  

const int SAMPLE_RATE_HZ = 512;
const int SAMPLE_PERIOD_US = 1000000 / SAMPLE_RATE_HZ;
unsigned long last_sample_time = 0;

int button_state = 0;
unsigned long last_blink = 0;
bool led_state = false;

// Filter Structure to keep states independent per channel
struct EMGChannelProcessor {
  float n1_z1 = 0, n1_z2 = 0;
  float n2_z1 = 0, n2_z2 = 0;
  float emg_z1 = 0, emg_z2 = 0;
  
  // Envelope State
  static const int ENV_WINDOW = (100 * SAMPLE_RATE_HZ) / 1000;
  float envBuffer[ENV_WINDOW] = {0};
  int envIndex = 0;
  float envSum = 0;

  float process(float raw_input) {
    // 1. Notch Filter
    float n_out = raw_input;
    float x1 = n_out - (-1.58696045 * n1_z1) - (0.96505858 * n1_z2);
    n_out = (0.96588529 * x1) + (-1.57986211 * n1_z1) + (0.96588529 * n1_z2);
    n1_z2 = n1_z1; n1_z1 = x1;

    float x2 = n_out - (-1.62761184 * n2_z1) - (0.96671306 * n2_z2);
    n_out = (1.00000000 * x2) + (-1.63566226 * n2_z1) + (1.00000000 * n2_z2);
    n2_z2 = n2_z1; n2_z1 = x2;

    // 2. EMG High/Band-pass Filter
    float e_out = n_out;
    float x3 = e_out - (-0.85080258 * emg_z1) - (-0.30256882 * emg_z2);
    e_out = (0.53834285 * x3) + (-1.07668570 * emg_z1) + (0.53834285 * emg_z2);
    emg_z2 = emg_z1; emg_z1 = x3;

    return e_out;
  }

  float getEnvelope(float filtered_emg) {
    float absSample = fabs(filtered_emg);
    envSum -= envBuffer[envIndex];
    envSum += absSample;
    envBuffer[envIndex] = absSample;
    envIndex = (envIndex + 1) % ENV_WINDOW;
    return envSum / ENV_WINDOW;
  }
};

EMGChannelProcessor jawProc, chinProc, earProc;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_JAW, INPUT);
  pinMode(PIN_CHIN, INPUT);
  pinMode(PIN_EAR, INPUT);
  pinMode(PIN_BTN, INPUT_PULLDOWN); 
  pinMode(LED_PIN, OUTPUT);
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  if (!client.connected()) {
    client = server.available();
    if (millis() - last_blink > 500) {
      led_state = !led_state;
      digitalWrite(LED_PIN, led_state);
      last_blink = millis();
    }
    return;
  }

  unsigned long now = micros();
  if (now - last_sample_time >= SAMPLE_PERIOD_US) {
    last_sample_time = now;
    
    float raw_jaw = analogRead(PIN_JAW);
    float raw_chin = analogRead(PIN_CHIN);
    float raw_ear = analogRead(PIN_EAR);
    
    button_state = digitalRead(PIN_BTN);
    if (button_state == 1) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      if (now / 1000 - last_blink > 100) {
        led_state = !led_state;
        digitalWrite(LED_PIN, led_state);
        last_blink = now / 1000;
      }
    }

    // Process Filters
    float f_jaw = jawProc.process(raw_jaw);
    float f_chin = chinProc.process(raw_chin);
    float f_ear = earProc.process(raw_ear);

    // Get Envelopes (Cleanest signal for ML)
    float env_jaw = jawProc.getEnvelope(f_jaw);
    float env_chin = chinProc.getEnvelope(f_chin);
    float env_ear = earProc.getEnvelope(f_ear);

    // Stream the envelope values directly matching the original code's preference
    String dataStr = String(millis()) + "," + 
                     String(env_jaw, 3) + "," + 
                     String(env_chin, 3) + "," + 
                     String(env_ear, 3) + "," + 
                     String(button_state) + "\n";
                     
    client.print(dataStr);
  }
}
