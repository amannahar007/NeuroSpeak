const socket = io('http://localhost:5000');

// UI Elements
const socketStatusDot = document.querySelector('#socket-status .dot');
const socketStatusText = document.querySelector('#socket-status .text');
const espStatusDot = document.querySelector('#esp-status .dot');
const espStatusText = document.querySelector('#esp-status .text');

const liveWord = document.getElementById('live-word');
const liveConfidence = document.getElementById('live-confidence');

const recStatus = document.getElementById('rec-status');
const recStatusSpan = document.querySelector('#rec-status span');

const valJaw = document.getElementById('val-jaw');
const valChin = document.getElementById('val-chin');
const valEar = document.getElementById('val-ear');

const logOutput = document.getElementById('log-output');

const labelInput = document.getElementById('label-input');
const saveLabelBtn = document.getElementById('save-label-btn');
const trainBtn = document.getElementById('train-model-btn');

// Chart Setup
const ctx = document.getElementById('emgChart').getContext('2d');
const MAX_POINTS = 100;
const emgChart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: Array(MAX_POINTS).fill(''),
        datasets: [
            {
                label: 'Jaw',
                data: Array(MAX_POINTS).fill(0),
                borderColor: '#00e5ff',
                borderWidth: 1.5,
                pointRadius: 0,
                tension: 0.2
            },
            {
                label: 'Chin',
                data: Array(MAX_POINTS).fill(0),
                borderColor: '#b026ff',
                borderWidth: 1.5,
                pointRadius: 0,
                tension: 0.2
            },
            {
                label: 'Ear',
                data: Array(MAX_POINTS).fill(0),
                borderColor: '#e2e8f0',
                borderWidth: 1.5,
                pointRadius: 0,
                tension: 0.2
            }
        ]
    },
    options: {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        plugins: { legend: { display: false } },
        scales: {
            x: { display: false },
            y: { 
                display: true, 
                grid: { color: 'rgba(255,255,255,0.05)' },
                border: { dash: [4, 4] },
                ticks: { color: '#718096', font: { family: 'JetBrains Mono' } }
            }
        }
    }
});

// Helper Function: Add Log
function addLog(msg, type = 'system') {
    const div = document.createElement('div');
    div.className = `log-entry ${type}`;
    const time = new Date().toLocaleTimeString([], { hour12: false });
    div.innerHTML = `[${time}] ${msg}`;
    logOutput.appendChild(div);
    logOutput.scrollTop = logOutput.scrollHeight;
}

// Socket Connection Events
socket.on('connect', () => {
    socketStatusDot.classList.add('connected');
    socketStatusText.innerText = 'Connected';
    addLog('Dashboard connected to Backend', 'system');
});

socket.on('disconnect', () => {
    socketStatusDot.classList.remove('connected');
    socketStatusText.innerText = 'Disconnected';
    addLog('Dashboard disconnected', 'error');
});

// Custom Events
socket.on('system_status', (data) => {
    if (data.esp_connected !== undefined) {
        if (data.esp_connected) {
            espStatusDot.classList.add('connected');
            espStatusText.innerText = 'Streaming';
        } else {
            espStatusDot.classList.remove('connected');
            espStatusText.innerText = 'Disconnected';
        }
    }
    
    if (data.recording !== undefined) {
        if (data.recording) {
            recStatus.classList.add('active');
            recStatusSpan.innerText = 'RECORDING ACTIVE';
        } else {
            recStatus.classList.remove('active');
            recStatusSpan.innerText = 'IDLE';
        }
    }
});

socket.on('live_data', (data) => {
    // Update text readouts
    valJaw.innerText = data.jaw.toFixed(2);
    valChin.innerText = data.chin.toFixed(2);
    valEar.innerText = data.ear.toFixed(2);
    
    // Update chart
    emgChart.data.datasets[0].data.shift();
    emgChart.data.datasets[0].data.push(data.jaw);
    
    emgChart.data.datasets[1].data.shift();
    emgChart.data.datasets[1].data.push(data.chin);
    
    emgChart.data.datasets[2].data.shift();
    emgChart.data.datasets[2].data.push(data.ear);
    
    emgChart.update();
});

socket.on('log', (data) => {
    addLog(data.msg, data.type || 'system');
});

socket.on('word_detected', (data) => {
    liveWord.innerText = data.word;
    liveConfidence.innerText = data.confidence;
    
    // Visual flash
    liveWord.style.color = '#00ff9d';
    setTimeout(() => {
        liveWord.style.color = '';
    }, 500);
    
    addLog(`DETECTED: ${data.word} (${data.confidence}%)`, 'detect');
});

// API Calls
saveLabelBtn.addEventListener('click', async () => {
    const label = labelInput.value.trim();
    if (!label) return alert("Please enter a label.");
    
    try {
        const res = await fetch('http://localhost:5000/save-label', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ label })
        });
        const data = await res.json();
        if (data.status === 'success') {
            addLog(`Saved recording to /dataset/${label}`, 'system');
            labelInput.value = '';
        } else {
            addLog(`Error saving: ${data.message}`, 'error');
            alert(data.message);
        }
    } catch (e) {
        addLog(`Fetch error: ${e.message}`, 'error');
    }
});

trainBtn.addEventListener('click', async () => {
    try {
        addLog(`Starting ML model retraining...`, 'system');
        const res = await fetch('http://localhost:5000/train-model', { method: 'POST' });
        const data = await res.json();
        if (data.status === 'success') {
            addLog(`Model Trained: ${data.message}`, 'system');
            alert("Training complete!");
        } else {
            addLog(`Training Error: ${data.message}`, 'error');
        }
    } catch (e) {
        addLog(`Fetch error: ${e.message}`, 'error');
    }
});
