#include <WiFi.h>

// WiFi Settings
const char* ssid = "Nahar";
const char* password = "your_password_here"; // Placeholder
const int tcp_port = 8080;

WiFiServer server(tcp_port);
WiFiClient client;

// Pin Definitions for ESP32-C6 (ADC1 mapped to GPIO0-6 usually)
#define PIN_JAW 0
#define PIN_CHIN 1
#define PIN_EAR 2
#define PIN_BTN 5  // Assuming A5 maps to GPIO5
#define LED_PIN 8  // Built-in LED on C6 (can vary by board, often 8 or 15)

// Timing
const int SAMPLE_RATE_HZ = 512;
const int SAMPLE_PERIOD_US = 1000000 / SAMPLE_RATE_HZ;
unsigned long last_sample_time = 0;

// Filter Structure
class Biquad {
  public:
    float b0, b1, b2, a1, a2;
    float x1, x2, y1, y2;

    Biquad() {
      x1 = x2 = y1 = y2 = 0;
    }

    void setNotch(float fs, float f0, float q) {
      float w0 = 2.0 * PI * f0 / fs;
      float alpha = sin(w0) / (2.0 * q);
      float a0 = 1.0 + alpha;
      b0 = 1.0 / a0;
      b1 = -2.0 * cos(w0) / a0;
      b2 = 1.0 / a0;
      a1 = -2.0 * cos(w0) / a0;
      a2 = (1.0 - alpha) / a0;
    }

    void setHighPass(float fs, float f0, float q) {
      float w0 = 2.0 * PI * f0 / fs;
      float alpha = sin(w0) / (2.0 * q);
      float a0 = 1.0 + alpha;
      b0 = ((1.0 + cos(w0)) / 2.0) / a0;
      b1 = -(1.0 + cos(w0)) / a0;
      b2 = ((1.0 + cos(w0)) / 2.0) / a0;
      a1 = -2.0 * cos(w0) / a0;
      a2 = (1.0 - alpha) / a0;
    }

    float process(float x) {
      float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
      x2 = x1;
      x1 = x;
      y2 = y1;
      y1 = y;
      return y;
    }
};

Biquad jaw_notch, jaw_hp;
Biquad chin_notch, chin_hp;
Biquad ear_notch, ear_hp;

int button_state = 0;
unsigned long last_blink = 0;
bool led_state = false;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_JAW, INPUT);
  pinMode(PIN_CHIN, INPUT);
  pinMode(PIN_EAR, INPUT);
  pinMode(PIN_BTN, INPUT_PULLDOWN); // Assume push triggers HIGH
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize Filters
  jaw_notch.setNotch(SAMPLE_RATE_HZ, 50.0, 1.0);
  jaw_hp.setHighPass(SAMPLE_RATE_HZ, 70.0, 0.707);
  
  chin_notch.setNotch(SAMPLE_RATE_HZ, 50.0, 1.0);
  chin_hp.setHighPass(SAMPLE_RATE_HZ, 70.0, 0.707);
  
  ear_notch.setNotch(SAMPLE_RATE_HZ, 50.0, 1.0);
  ear_hp.setHighPass(SAMPLE_RATE_HZ, 70.0, 0.707);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("TCP Server started on port 8080");
}

void loop() {
  // Handle Client Connection
  if (!client.connected()) {
    client = server.available();
    if (millis() - last_blink > 500) {
      led_state = !led_state;
      digitalWrite(LED_PIN, led_state);
      last_blink = millis();
    }
    return;
  }

  // Once connected, do strict sampling
  unsigned long now = micros();
  if (now - last_sample_time >= SAMPLE_PERIOD_US) {
    last_sample_time = now;
    
    // Read raw ADC
    int raw_jaw = analogRead(PIN_JAW);
    int raw_chin = analogRead(PIN_CHIN);
    int raw_ear = analogRead(PIN_EAR);
    
    // Debounced button read directly inline for speed
    button_state = digitalRead(PIN_BTN);
    
    // Handle LED State
    if (button_state == 1) {
      digitalWrite(LED_PIN, HIGH); // Solid when recording
    } else {
      if (now / 1000 - last_blink > 100) {
        led_state = !led_state;
        digitalWrite(LED_PIN, led_state); // Fast blink when connected
        last_blink = now / 1000;
      }
    }

    // Apply Filters
    float out_jaw = jaw_hp.process(jaw_notch.process((float)raw_jaw));
    float out_chin = chin_hp.process(chin_notch.process((float)raw_chin));
    float out_ear = ear_hp.process(ear_notch.process((float)raw_ear));

    // Send via TCP (Format: timestamp_ms,jaw,chin,ear,button_state)
    String dataStr = String(millis()) + "," + 
                     String(out_jaw, 2) + "," + 
                     String(out_chin, 2) + "," + 
                     String(out_ear, 2) + "," + 
                     String(button_state) + "\n";
                     
    client.print(dataStr);
  }
}
