# NeuroSpeak - ESP32-C6 Silent Speech AI 🧠

A complete, Local ML-powered EMG Silent Speech recognition system leveraging the ESP32-C6, Python FastAPI, and a State-of-the-Art Sci-Fi Web Dashboard.

## Architecture
1. **ESP32-C6 (Hardware):** Reads 3 EMG analog channels at 512Hz. High-pass and Notch filters process the signal. TCP Server streams data pointlessly fast. Button on A5 toggles Dataset Recording.
2. **Backend (Python):** Connects to the ESP32-C6 via TCP socket. Handles the sliding 512-sample window buffer. Triggers Real-Time ML inference locally. Pushes verified results strictly to Firebase for cloud sync.
3. **Web Dashboard:** Complex Vanilla CSS Glassmorphism dashboard listening on WebSocket events from the Backend. Features live signal reading, visual ML detections, and label configuration.

## Folder Structure
- `/esp32_firmware/` - The `.ino` script for the ESP32-C6. 
- `/backend/` - FastAPI + SocketIO + ML logic. 
- `/dashboard/` - HTML, CSS, JS. 
- `/data/dataset/` - Saved `.npy` numpy arrays for training!

## Step 1: ESP32-C6 Setup
1. Open `esp32_firmware.ino` in Arduino IDE.
2. Ensure you have the ESP32-C6 board package installed.
3. Change the WiFi `ssid` and `password`.
4. Flash the code. The ESP32 will boot and print out its `IP Address` over Serial.

## Step 2: Backend Setup
1. Open the `/backend/` folder in your terminal.
2. Create `venv` and activate it.
3. Install reqs: `pip install -r requirements.txt`
4. Create a `.env` file in the backend folder:
   ```env
   ESP_IP="YOUR.ESP.IP.ADDRESS"
   ESP_PORT=8080
   FIREBASE_CRED_PATH="path/to/cred.json"
   FIREBASE_DB_URL="https://your-db.firebaseio.com"
   ```
5. Run the Backend: `python app.py`

## Step 3: Web Dashboard Setup
1. Simply double-click `dashboard/index.html` in your browser. (Since CORS is enabled in the backend, you can run it as a file).
2. The Dashboard will instantly connect to the Python Backend and display the ESP32 sensor state.

## Step 4: Machine Learning Pipeline
1. **Record Data:** Press the A5 Button on the ESP32-C6. The dashboard will flash "RECORDING ACTIVE". Hold the muscle contraction (e.g., saying "Help" silently). Let go of the button.
2. **Label Data:** On the Dashboard, type "Help" and click **Save Label**.
3. **Train Model:** Once you have a few words, click **Retrain ML Model** on the dashboard.
4. **Live Inference:** The ML model is now active. When not recording, the system will evaluate the 512-length window every second to see if you trigger a threshold. If matched, it outputs the word!

## Edge Cases Guarded
- **No data over TCP?** The backend handles reconnects automatically every 2 seconds.
- **Over-recording?** Signals are bounded, and you apply labels post-action to ensure data gets validated before it hits the `data/dataset` folder.
- **Cloud isolation:** The ESP32 does *not* talk to Firebase, preventing device compromise and latency. Firebase is updated only by the secure Python backend on verified word detections.
