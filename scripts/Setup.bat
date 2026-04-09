@echo off
pushd %~dp0
echo ============================================
echo   Bolt Engine Setup
echo ============================================
echo.
python --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python is not installed or not in PATH.
    echo         Download from https://www.python.org/downloads/
    PAUSE
    exit /b 1
)
python Setup.py
PAUSE
