import os
import numpy as np
import time

DATA_DIR = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'data', 'dataset')

def ensure_dir(path):
    if not os.path.exists(path):
        os.makedirs(path)

def save_recording(label: str, data: np.ndarray):
    """
    Saves a 3-channel recording buffer to a label directory.
    data format: shape (N, 3) -> Jaw, Chin, Ear
    """
    ensure_dir(DATA_DIR)
    label_dir = os.path.join(DATA_DIR, label.lower())
    ensure_dir(label_dir)
    
    timestamp = int(time.time() * 1000)
    filename = f"sample_{timestamp}.npy"
    filepath = os.path.join(label_dir, filename)
    
    np.save(filepath, data)
    return filepath

def load_dataset():
    """
    Loads all .npy files from the dataset directory.
    Returns:
        X: List of numpy arrays 
        y: List of string labels
    """
    X = []
    y = []
    
    if not os.path.exists(DATA_DIR):
        return X, y
        
    for label in os.listdir(DATA_DIR):
        label_dir = os.path.join(DATA_DIR, label)
        if os.path.isdir(label_dir):
            for file in os.listdir(label_dir):
                if file.endswith('.npy'):
                    filepath = os.path.join(label_dir, file)
                    data = np.load(filepath)
                    # We might want to pad or truncate here if sizes vary, 
                    # but typically they are 1-sec windows (512 samples)
                    X.append(data)
                    y.append(label)
                    
    return X, y
