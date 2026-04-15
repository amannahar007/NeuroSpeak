# Goal Description
Build a complete, real-time EMG silent speech system utilizing an ESP32-C6 to capture 3-channel EMG data at 512 Hz. Develop a Python backend to handle continuous ML inference and dataset recording (triggered by a button on A5). Finally, create a vibrant, beautiful Web Dashboard for displaying real-time metrics, detection logs, and handling dataset tagging.

## User Review Required
Please review the system architecture and flow as described below. If this looks complete and accurate to the requirements, please approve and we will proceed to execution.

> [!WARNING]
> The exact pin used for the ESP32-C6 analog reads will depend on the ADC capabilities of the C6 mapping. Standard ESP32 uses pins like 32, 34, 35, but on ESP32-C6, ADC1 channels map to GPIO0-GPIO6. I will map Jaw, Chin, Ear to valid ADC pins on ESP32-C6 and A5 to GPIO5 (or a suitable button pin). Does this setup work for your ESP32-C6 module?

## Proposed Changes

### Configuration & Settings
#### [NEW] .env
Will contain configuration limits, Firebase credentials path (for word logging only), and other settings to run the backend correctly.

---

### ESP32-C6 Embedded Firmware
#### [NEW] esp32_firmware/esp32_firmware.ino
- Setup WiFi TCP Server on Port 8080.
- Configure ADC to sample 3 EMG channels (Jaw, Chin, Ear) and 1 digital button state (A5) via hardware timer at 512Hz.
- Implement efficient IIR 50Hz Notch Filter and 70Hz High-pass Filter using standard math.
- Stream CSV structured data: `timestamp,jaw,chin,ear,button_state`
- LED blinking to indicate connection vs recording.

---

### Python Backend Service
#### [NEW] backend/app.py
- Use FastAPI + SocketIO (`python-socketio` with ASGI).
- **TCP Stream Manager**: Connect to ESP32 TCP server, handle auto-reconnecting, parse `timestamp,jaw,chin,ear,button_state`.
- **Inference Pipeline**: Maintain a 512-sample circular buffer. Check confidence, cooldown, trigger Firebase logging via `firebase-admin`, and broadcast prediction via WebSocket.
- **Training Pipeline**: Detect button 0->1 (start buffer), 1->0 (stop and save buffer to `data/temp_recording.npy`).
- **REST APIs**: `/status`, `/ping-esp`, `/save-label`, `/train-model`.

#### [NEW] backend/ml_models.py
- Methods for Random Forest / SVM window feature extraction (RMS, MAV, peak, variance).
- Methods for 1D CNN training using Keras/PyTorch.
- Training loops triggered via FastAPI endpoint.

#### [NEW] backend/data_manager.py
- Handle dataset organization: `/dataset/<label>/sample_X.npy`.
- Provide Kaggle external dataset import functions.

#### [NEW] backend/requirements.txt
- Document backend packages required for the project.

---

### Web Dashboard
#### [NEW] dashboard/index.html
- Beautiful, modern web interface featuring a dark, premium aesthetic with gradients, micro-animations, and glassmorphism.
- Panels for Connection Status, Recording Status, Label Assignment, Live Detection, Debug Signals, and Logs.

#### [NEW] dashboard/style.css
- Complex Vanilla CSS using variables, animations, and custom typography (e.g. Inter).

#### [NEW] dashboard/script.js
- WebSocket connections to update the UI efficiently.
- Form submissions to `/save-label`.
- Fetch calls to `/train-model` and `/ping-esp`.

---

### Management & Documentation
#### [NEW] design.md
- Document the design system and aesthetic choices for the Web Dashboard in the root directory.

#### [NEW] task.md
- Document the breakdown of components per the USER's instructions in the root directory.

#### [NEW] README.md
- Comprehensive setup guide for the ESP32 code, Backend, Dashboard, and ML training.

## Verification Plan

### Automated/Local Tests
- Run Python backend and mock ESP32 TCP stream to test the real-time buffer processing and model trigger logic.
- Run FastAPI tests for endpoints.

### Manual Verification
- Flash `esp32_firmware.ino` to ESP32-C6.
- Load the Web Dashboard, observe connection status.
- Press A5 to record data, save labels in Dashboard.
- Train the ML pipeline via the Dashboard.
- Activate real-time prediction and observe outputs logging to Dashboard and Firebase.
