# 🧠 NeuroSpeak Developer Traceability Guide

This document is for developers pushing or maintaining the NeuroSpeak project. It tracks the core logic and recent enhancements.

## 🏗 System Architecture
1. **Frontend**: Vanilla HTML/JS with Chart.js for visualization. Communicates via SocketIO for live data and REST API for training/saving.
2. **Backend**: FastAPI (Python) server. Handles TCP connection to ESP32 and runs Machine Learning (Random Forest).
3. **Firmware**: Arduino/ESP32 code to read EMG sensors and stream data.

## 📡 Signal Processing Flow
1. **Bio-Signals**: ESP32 reads 3 analog channels -> Notch Filter (50Hz) -> Envelope detection -> TCP Stream.
2. **Feature Extraction**: Backend takes 512-sample windows and calculates **RMS**, **MAV**, **Variance**, and **Peak** to identify speech patterns.
3. **Classification**: `RandomForestClassifier` trained on `.npy` samples compares live features against the dataset.

## 🔄 Recent Changes (Traceability)

### 1. Manual Recording Integration
- **Why**: To allow recording samples directly from the dashboard UI without needing a physical button on the ESP32.
- **Affected Files**:
    - `backend/app.py`: Added `manual_recording` state and `/toggle-recording` endpoint.
    - `dashboard/index.html`: Added "Start/Stop Recording" button.
    - `dashboard/script.js`: Integrated toggle logic and visual UI updates.

### 2. Dataset Management
- **Directory**: Created `data/dataset/` to ensure a consistent path for all `.npy` samples across different machines.
- **Logic**: Labels typed in the UI dynamically generate subfolders in this directory.

## 🧪 How to Add New Words
1. Open the Dashboard.
2. Type a word in the label box (e.g., `water`).
3. Click **Start Recording**, perform the movement, and click **Stop Recording**.
4. Click **Save Label** to store the `.npy` file.
5. Record at least 15 samples of the word + 15 samples of `idle`.
6. Click **Retrain ML Model**.

---
*Maintained by AI Coding Assistant - NeuroSpeak Automation Layer*
