@echo off
setlocal enableextensions

REM ============================================================
REM  PortionOfScreen - Download & Run
REM  Downloads the latest .exe from the repo and launches it.
REM ============================================================

set "REPO_OWNER=BeSmarterWithData"
set "REPO_NAME=PortionOfScreen"
set "BRANCH=main"
set "EXE_NAME=PortionOfScreen.exe"
set "EXE_URL=https://raw.githubusercontent.com/%REPO_OWNER%/%REPO_NAME%/%BRANCH%/bin/%EXE_NAME%"

REM Install to user-local directory (no admin rights needed)
set "INSTALL_DIR=%LOCALAPPDATA%\PortionOfScreen"

mkdir "%INSTALL_DIR%" >nul 2>&1

echo [INFO] PortionOfScreen Installer / Launcher
echo [INFO] Install directory: "%INSTALL_DIR%"
echo.

REM ============================================================
REM  Download the latest .exe from the repo
REM ============================================================
echo [INFO] Downloading latest %EXE_NAME%...

where curl >nul 2>&1
if %errorlevel%==0 (
    curl -L -o "%INSTALL_DIR%\%EXE_NAME%" "%EXE_URL%"
) else (
    powershell -NoProfile -Command "Invoke-WebRequest -Uri '%EXE_URL%' -OutFile '%INSTALL_DIR%\%EXE_NAME%' -UseBasicParsing"
)

if not exist "%INSTALL_DIR%\%EXE_NAME%" (
    echo [ERROR] Failed to download %EXE_NAME%.
    echo [ERROR] URL: %EXE_URL%
    pause
    exit /b 1
)

echo [INFO] Downloaded successfully.

REM ============================================================
REM  Launch
REM ============================================================
:launch
echo.
echo [INFO] Launching PortionOfScreen...
start "" "%INSTALL_DIR%\%EXE_NAME%"
echo [INFO] Done.
exit /b 0
