# NeuroSpeak Project Tasks

- `[x]` **Phase 1: Project Setup & Firmware**
  - `[x]` Setup standard folder structure (`esp32_firmware`, `backend`, `dashboard`, `data`)
  - `[x]` Copy previous dataset artifacts to `data`
  - `[x]` Develop ESP32-C6 `.ino` for 3-channel analog read at 512 Hz
  - `[x]` Implement 50Hz Notch + 70Hz High-pass math on ESP32
  - `[x]` Integrate Button A5 logical state processing on ESP32
  - `[x]` Configure TCP Server at port 8080 on ESP32 for raw string streaming

- `[x]` **Phase 2: Python Backend**
  - `[x]` Initialize FastAPI / Flask SocketIO setup
  - `[x]` Build TCP client to auto-connect to ESP32
  - `[x]` Build ring buffer parser (size 512) for handling streaming CSV data
  - `[x]` Implement training data recording logic based on `button_state` 0/1 flips
  - `[x]` Design data storage structures (save vectors to `/dataset/<label>/<file>.npy`)

- `[x]` **Phase 3: Machine Learning Models**
  - `[x]` Create RMS/MAV feature extraction helper functions
  - `[x]` Implement Random Forest / SVM baseline logic
  - `[x]` Implement standard 1D CNN layout in preferred library
  - `[x]` Expose `/train-model` API wrapper for retraining
  - `[x]` Connect inference pipeline (check thresholds -> check confidence -> apply cooldown)

- `[x]` **Phase 4: Web Dashboard**
  - `[x]` Implement index.html with base structural layout
  - `[x]` Apply `design.md` visual logic with complex Vanilla CSS
  - `[x]` Connect client-side WebSocket parsing
  - `[x]` Add components: Connection Status, Recording Trigger Visuals, Live Word Detection, Debug Live Graphs
  - `[x]` Bind "Save Label" UI to actual backend endpoint

- `[x]` **Phase 5: Cloud Integration & Final Review**
  - `[x]` Bind Firebase API in backend for final predictions strictly (no raw signal uploading)
  - `[x]` Document final `README.md`
  - `[x]` Verify everything locally, resolve edge cases (debounce, disconnection)
  - `[x]` Final push to `https://github.com/amannahar007/NeuroSpeak`
