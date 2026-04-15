import numpy as np
import joblib
import os
from sklearn.ensemble import RandomForestClassifier
from data_manager import load_dataset

MODEL_PATH = os.path.join(os.path.dirname(__file__), 'rf_model.pkl')

def extract_features_from_window(window_data):
    """
    Extracts features for Machine Learning from a (N, 3) matrix.
    Using RMS, Mean Absolute Value (MAV), and Variance per channel.
    """
    features = []
    # window_data shape is (512, 3) representing (Jaw, Chin, Ear)
    for channel in range(window_data.shape[1]):
        channel_data = window_data[:, channel]
        
        rms = np.sqrt(np.mean(channel_data**2))
        mav = np.mean(np.abs(channel_data))
        variance = np.var(channel_data)
        peak = np.max(np.abs(channel_data))
        
        features.extend([rms, mav, variance, peak])
        
    return np.array(features)

def train_baseline_model():
    """
    Loads dataset, extracts features, trains RF, saves model.
    """
    X_raw, y = load_dataset()
    
    if len(X_raw) == 0:
        return {"status": "error", "message": "No dataset found. Record data first."}
        
    # Extract features
    X_features = []
    for window in X_raw:
        # If lengths are weird, we could truncate, but assuming roughly fixed size
        X_features.append(extract_features_from_window(window))
        
    X_features = np.array(X_features)
    y = np.array(y)
    
    # Train random forest
    clf = RandomForestClassifier(n_estimators=100, random_state=42)
    clf.fit(X_features, y)
    
    # Save model
    joblib.dump(clf, MODEL_PATH)
    
    return {"status": "success", "message": f"Model trained on {len(X_raw)} samples. Classes: {clf.classes_}"}

def load_inference_model():
    if os.path.exists(MODEL_PATH):
        return joblib.load(MODEL_PATH)
    return None

def predict_window(model, window_data):
    """
    Returns prediction and confidence for a raw window.
    """
    features = extract_features_from_window(window_data).reshape(1, -1)
    
    probabilities = model.predict_proba(features)[0]
    best_idx = np.argmax(probabilities)
    confidence = probabilities[best_idx]
    prediction = model.classes_[best_idx]
    
    return prediction, confidence
