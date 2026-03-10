# Portion of Screen
## Share a portion of your screen in Microsoft Teams or Google Meet
_Portion of Screen_ is a lightweight Windows application that enables you to share a part of your screen in video conferencing apps that only support full screen or just a single window.

### Quick Install & Run
Download and run [`PortionOfScreen.bat`](PortionOfScreen.bat) — it will automatically download the latest `PortionOfScreen.exe` to `%LOCALAPPDATA%\PortionOfScreen` and launch it. No admin rights required.

Alternatively, download `PortionOfScreen.exe` directly from the [`dist`](dist/) folder or from [Releases](https://github.com/egonl/PortionOfScreen/releases).

### Usage in Microsoft Teams
1. Start _PortionOfScreen.exe_ (or run the `.bat` file).
2. In a Teams meeting, select **Share** > **Window** > **Portion of Screen**.

_Portion of Screen_ supports two modes: 
- _Focus Mode_ (default): When Focus Mode is enabled, the window that currently has the focus will automatically be shared. Use this mode if you're regularly switching between windows.
- _Fixed Mode_: Only the area defined by the _Portion of Screen_ window will be shared. You can resize and move this window while presenting. Send the window to the background by left clicking it.

The operating mode can be selected using **System Menu** > **Options**.

### Building from Source
Open `PortionOfScreen.sln` in Visual Studio and build the Release x64 configuration.

### Notes
- You (the presenter) won't see the _Portion of Screen_ window if it doesn't have the focus. However, your audience will see the _Portion of Screen_ window.
- You can send the _Portion of Screen_ window to the background by left clicking it.
- If you close the _Portion of Screen_ window, Teams will automatically stop sharing.
