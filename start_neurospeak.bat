@echo off
echo =========================================
echo    Starting NeuroSpeak System...
echo =========================================

cd /d "%~dp0"

echo [1/3] Checking Python Backend Environment...
cd backend

IF NOT EXIST "venv\Scripts\activate.bat" (
    echo Creating virtual environment (this might take a second...^)
    python -m venv venv
)

echo Activating virtual environment...
call venv\Scripts\activate.bat

echo Installing dependencies from requirements.txt...
pip install -r requirements.txt | findstr /i /v "already satisfied"

echo [2/3] Starting Backend Server...
:: Starts the python server in a new dedicated Command Prompt window
start "NeuroSpeak Backend Service" cmd /k "title NeuroSpeak Backend FastApi & call venv\Scripts\activate.bat & python app.py"

:: Wait exactly 3 seconds for the backend to spin up before blasting the UI
timeout /t 3 /nobreak > nul

echo [3/3] Launching Web Dashboard...
cd ..
:: Open the UI directly in the default internet browser
start "" "%~dp0dashboard\index.html"

echo =========================================
echo    System Online! 
echo    You can see the backend logs in the new terminal window.
echo    Close that window to stop the backend.
echo =========================================
pause
