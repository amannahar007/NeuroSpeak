import asyncio
import os
import time
import numpy as np
from fastapi import FastAPI, BackgroundTasks, Request
from fastapi.middleware.cors import CORSMiddleware
import socketio
from dotenv import load_dotenv

import firebase_admin
from firebase_admin import credentials, db

from data_manager import save_recording
from ml_models import train_baseline_model, load_inference_model, predict_window

load_dotenv()

# Firebase Config (Optional depending on .env)
firebase_enabled = False
try:
    cred_path = os.getenv("FIREBASE_CRED_PATH", "firebase_cred.json")
    if os.path.exists(cred_path):
        cred = credentials.Certificate(cred_path)
        firebase_admin.initialize_app(cred, {
            'databaseURL': os.getenv("FIREBASE_DB_URL", "")
        })
        firebase_enabled = True
        print("✓ Firebase initialized")
except Exception as e:
    print(f"⚠ Firebase disabled: {e}")

# FastAPI and SocketIO Setup
app = FastAPI()
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

sio = socketio.AsyncServer(async_mode='asgi', cors_allowed_origins='*')
socket_app = socketio.ASGIApp(sio, app)
app.mount("/ws", socket_app)

# Global State
class SystemState:
    def __init__(self):
        self.esp_connected = False
        self.is_recording = False
        self.emg_buffer = [] # Shape: maxlen 512, inner: [jaw, chin, ear]
        self.recording_buffer = []
        self.model = load_inference_model()
        self.last_detection_time = 0
        self.cooldown_sec = 2.0
        self.min_energy_threshold = 0.015 # Minimal logic to trigger

state = SystemState()

async def push_to_firebase(prediction, confidence, timestamp):
    if not firebase_enabled:
        return
    try:
        ref = db.reference('/detected_words')
        ref.push({
            'word': prediction,
            'confidence': float(confidence),
            'timestamp': timestamp,
        })
    except Exception as e:
        print(f"Firebase Push Error: {e}")

async def process_inference_window(window_data):
    # window_data shape is (512, 3)
    if state.model is None:
        return
        
    # Energy Gate Check vs Jaw/Chin/Ear
    # Quick RMS on channel 0
    rms = np.sqrt(np.mean(window_data[:,0]**2) + 1e-6)
    if rms < state.min_energy_threshold:
        return # Ignore idle noise
        
    prediction, confidence = predict_window(state.model, window_data)
    
    if confidence > 0.85 and prediction.lower() != 'idle':
        current_time = time.time()
        if (current_time - state.last_detection_time > state.cooldown_sec):
            state.last_detection_time = current_time
            print(f"🧠 Detected: {prediction} ({confidence*100:.1f}%)")
            
            payload = {
                'word': prediction,
                'confidence': f"{confidence*100:.1f}",
                'timestamp': current_time
            }
            await sio.emit('word_detected', payload)
            asyncio.create_task(push_to_firebase(prediction, confidence, current_time))
            # Clear buffer slightly to avoid double firing
            state.emg_buffer.clear()

async def tcp_client_task():
    reader, writer = None, None
    host = os.getenv("ESP_IP", "192.168.1.100")
    port = int(os.getenv("ESP_PORT", 8080))
    
    while True:
        try:
            if not state.esp_connected:
                print(f"Connecting to ESP32 at {host}:{port}...")
                reader, writer = await asyncio.open_connection(host, port)
                state.esp_connected = True
                await sio.emit('system_status', {'esp_connected': True})
                print("✓ ESP32 Connected")

            line = await reader.readline()
            if not line:
                raise ConnectionError("Connection closed by server")
                
            decoded = line.decode('utf-8').strip()
            parts = decoded.split(',')
            
            if len(parts) == 5:
                # Format: timestamp_ms,jaw,chin,ear,button_state
                timestamp, jaw, chin, ear, btn = parts
                jaw, chin, ear = float(jaw), float(chin), float(ear)
                btn = int(btn)
                
                # Live Data stream subset
                if len(state.emg_buffer) % 10 == 0:
                    await sio.emit('live_data', {'jaw': jaw, 'chin': chin, 'ear': ear})
                
                # Recording Logic
                if btn == 1:
                    if not state.is_recording:
                        state.is_recording = True
                        state.recording_buffer = []
                        await sio.emit('log', {'msg': 'Started Recording'})
                        await sio.emit('system_status', {'recording': True})
                    state.recording_buffer.append([jaw, chin, ear])
                else:
                    if state.is_recording:
                        state.is_recording = False
                        await sio.emit('log', {'msg': f'Stopped Recording. {len(state.recording_buffer)} samples.'})
                        await sio.emit('system_status', {'recording': False})
                        # Cache the most recent recording for label assignment
                        state.last_recorded_data = np.array(state.recording_buffer)
                
                # Inference buffer appending
                state.emg_buffer.append([jaw, chin, ear])
                if len(state.emg_buffer) > 512:
                    state.emg_buffer.pop(0)
                    
                if len(state.emg_buffer) == 512:
                    window = np.array(state.emg_buffer)
                    # Don't infer while recording
                    if not state.is_recording:
                        await process_inference_window(window)
                
        except Exception as e:
            print(f"TCP Error: {e}")
            state.esp_connected = False
            await sio.emit('system_status', {'esp_connected': False})
            if writer:
                writer.close()
                await writer.wait_closed()
            await asyncio.sleep(2) # Reconnect delay

@app.on_event("startup")
async def startup_event():
    asyncio.create_task(tcp_client_task())

@app.get("/status")
def get_status():
    return {
        "esp_connected": state.esp_connected,
        "model_loaded": state.model is not None,
        "firebase_enabled": firebase_enabled
    }

@app.post("/save-label")
async def api_save_label(request: Request):
    data = await request.json()
    label = data.get("label", "unknown")
    if hasattr(state, 'last_recorded_data') and state.last_recorded_data is not None and len(state.last_recorded_data) > 0:
        path = save_recording(label, state.last_recorded_data)
        state.last_recorded_data = None # consume it
        return {"status": "success", "file": path}
    return {"status": "error", "message": "No recording available."}

@app.post("/train-model")
async def api_train_model():
    res = train_baseline_model()
    if res['status'] == 'success':
        state.model = load_inference_model()
    return res

if __name__ == "__main__":
    import uvicorn
    uvicorn.run("app:app", host="0.0.0.0", port=5000, reload=True)
