@echo off
setlocal enableextensions

REM ============================================================
REM  PortionOfScreen - Download & Run
REM  Downloads the latest release from GitHub and launches it.
REM ============================================================

set "REPO_OWNER=BeSmarterWithData"
set "REPO_NAME=PortionOfScreen"
set "BRANCH=main"
set "EXE_NAME=PortionOfScreen.exe"

REM Install to user-local directory (no admin rights needed)
set "INSTALL_DIR=%LOCALAPPDATA%\PortionOfScreen"
set "TMP_DIR=%TEMP%\pos_%RANDOM%%RANDOM%"
set "ZIP_FILE=%TMP_DIR%\repo.zip"
set "UNZIP_DIR=%TMP_DIR%\unzipped"

mkdir "%TMP_DIR%" >nul 2>&1
mkdir "%INSTALL_DIR%" >nul 2>&1

echo [INFO] PortionOfScreen Installer / Launcher
echo [INFO] Install directory: "%INSTALL_DIR%"
echo.

REM ============================================================
REM  Step 1: Try downloading latest GitHub Release asset (.exe)
REM ============================================================
echo [INFO] Checking for latest GitHub Release...

set "RELEASE_URL="
powershell -NoProfile -Command ^
  "$ErrorActionPreference='Stop'; try { " ^
  "  $rel = Invoke-RestMethod -Uri 'https://api.github.com/repos/%REPO_OWNER%/%REPO_NAME%/releases/latest' -UseBasicParsing; " ^
  "  $asset = $rel.assets | Where-Object { $_.name -like '*.exe' } | Select-Object -First 1; " ^
  "  if ($asset) { " ^
  "    $url = $asset.browser_download_url; " ^
  "    Write-Output $url; " ^
  "    Invoke-WebRequest -Uri $url -OutFile '%INSTALL_DIR%\%EXE_NAME%' -UseBasicParsing; " ^
  "    Write-Host '[INFO] Downloaded release asset:' $asset.name; " ^
  "    exit 0 " ^
  "  } else { exit 1 } " ^
  "} catch { exit 1 }"

if %errorlevel%==0 (
    echo [INFO] Successfully downloaded latest release.
    goto :launch
)

echo [INFO] No release asset found. Downloading repository source...

REM ============================================================
REM  Step 2: Fallback - download repo ZIP and look for .exe
REM ============================================================
set "ZIP_URL=https://github.com/%REPO_OWNER%/%REPO_NAME%/archive/refs/heads/%BRANCH%.zip"

echo [INFO] Downloading from: %ZIP_URL%
where curl >nul 2>&1
if %errorlevel%==0 (
    curl -L -o "%ZIP_FILE%" "%ZIP_URL%"
) else (
    powershell -NoProfile -Command "Invoke-WebRequest -Uri '%ZIP_URL%' -OutFile '%ZIP_FILE%' -UseBasicParsing"
)

if not exist "%ZIP_FILE%" (
    echo [ERROR] Failed to download repository.
    pause
    goto :cleanup
)

echo [INFO] Extracting files...
powershell -NoProfile -Command "Expand-Archive -Path '%ZIP_FILE%' -DestinationPath '%UNZIP_DIR%' -Force"

REM Find the extracted top-level folder
for /d %%D in ("%UNZIP_DIR%\%REPO_NAME%-%BRANCH%") do set "TOPDIR=%%~fD"

REM Look for a pre-built .exe anywhere in the extracted repo
set "FOUND_EXE="
for /r "%TOPDIR%" %%F in (%EXE_NAME%) do (
    set "FOUND_EXE=%%~fF"
)

if defined FOUND_EXE (
    echo [INFO] Found pre-built executable: "%FOUND_EXE%"
    copy /y "%FOUND_EXE%" "%INSTALL_DIR%\%EXE_NAME%" >nul
    echo [INFO] Copied to: "%INSTALL_DIR%\%EXE_NAME%"
    goto :launch
)

REM No pre-built .exe found - copy the whole repo and try to build
echo [INFO] No pre-built .exe found in repo.
echo [INFO] Copying source files to install directory...
xcopy "%TOPDIR%\*" "%INSTALL_DIR%\" /e /i /y /q >nul

REM Check if MSBuild is available to compile
set "MSBUILD="
for %%M in (
    "%ProgramFiles%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
    "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    "%ProgramFiles%\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe"
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
) do (
    if exist %%M (
        set "MSBUILD=%%~M"
        goto :found_msbuild
    )
)

echo [ERROR] No pre-built .exe in the repo and MSBuild was not found.
echo [ERROR] To fix this, either:
echo          1. Add a compiled PortionOfScreen.exe to the GitHub Release assets
echo          2. Install Visual Studio Build Tools to compile from source
echo.
pause
goto :cleanup

:found_msbuild
echo [INFO] Found MSBuild: "%MSBUILD%"
echo [INFO] Building PortionOfScreen from source...

set "SLN_FILE=%INSTALL_DIR%\PortionOfScreen.sln"
if not exist "%SLN_FILE%" (
    echo [ERROR] Solution file not found: "%SLN_FILE%"
    pause
    goto :cleanup
)

"%MSBUILD%" "%SLN_FILE%" /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal
if %errorlevel% neq 0 (
    echo [ERROR] Build failed.
    pause
    goto :cleanup
)

REM Find the built .exe
for /r "%INSTALL_DIR%" %%F in (%EXE_NAME%) do (
    set "FOUND_EXE=%%~fF"
)

if defined FOUND_EXE (
    copy /y "%FOUND_EXE%" "%INSTALL_DIR%\%EXE_NAME%" >nul
    echo [INFO] Build successful.
    goto :launch
)

echo [ERROR] Build completed but .exe not found.
pause
goto :cleanup

REM ============================================================
REM  Launch
REM ============================================================
:launch
echo.
echo [INFO] Launching PortionOfScreen...
start "" "%INSTALL_DIR%\%EXE_NAME%"
echo [INFO] Done.

:cleanup
if exist "%TMP_DIR%" rmdir /s /q "%TMP_DIR%" >nul 2>&1
exit /b 0
