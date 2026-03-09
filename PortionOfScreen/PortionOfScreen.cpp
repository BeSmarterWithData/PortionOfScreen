// PortionOfScreen.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "PortionOfScreen.h"
#include <ShlObj.h>
#include <strsafe.h>

// Auto remove PoS caption by setting the WS_POPUP style in focused mode after one minute.
// However, WS_POPUP windows can't be shared, so you have to share within one minute after de-activating PoS.
//#define AUTO_REMOVE_CAPTION

#define MAX_LOADSTRING 100
#define IDT_REDRAW     101
#define IDC_OPTIONS    1100
#define POS_MIN_WIDTH  320
#define POS_MIN_HEIGHT 200
#define REDRAW_INTERVAL_MS 33
#define FOCUS_SWITCH_DELAY_MS 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
bool moveToDefaultWindowPos = false;
HWND newFocusHwnd;
unsigned int focusTimeMs;

// Global settings
bool focusMode =  true;
RECT defaultWindowPos;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                LoadSettings();
void                SaveSettings();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Prevent automatic scaling
    SetProcessDPIAware();

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PORTIONOFSCREEN, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PORTIONOFSCREEN));

    WNDCLASSEXW wcex;
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = hIcon;
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = hIcon;

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   LoadSettings();

   HWND hWnd = CreateWindowExW(
       WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
       szWindowClass,
       szTitle,
       WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX,
       defaultWindowPos.left,
       defaultWindowPos.top,
       defaultWindowPos.right - defaultWindowPos.left,
       defaultWindowPos.bottom - defaultWindowPos.top,
       nullptr,
       nullptr,
       hInstance,
       nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   HMENU hSysMenu = GetSystemMenu(hWnd, FALSE);
   AppendMenu(hSysMenu, MF_SEPARATOR, 0, NULL);
   AppendMenu(hSysMenu, MF_STRING, IDC_OPTIONS, L"Options");

   ShowWindow(hWnd, nCmdShow);
   SetLayeredWindowAttributes(hWnd, RGB(255, 255, 255), 128, LWA_ALPHA);
   UpdateWindow(hWnd);

    SetTimer(hWnd, IDT_REDRAW, REDRAW_INTERVAL_MS, (TIMERPROC)NULL);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
#ifdef AUTO_REMOVE_CAPTION
    case WM_NCACTIVATE:
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
        // Restore PoS caption
        SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX);
        return DefWindowProc(hWnd, message, wParam, lParam);
#endif

    case WM_LBUTTONUP:
        SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        break;

    case WM_SETFOCUS:
        if (focusMode)
        {
            // The window could be anywhere in Focus Mode, even over the taskbar. Quickly move window to it's original position.
            SetWindowPos(hWnd, HWND_TOPMOST, defaultWindowPos.left, defaultWindowPos.top, defaultWindowPos.right - defaultWindowPos.left, 0, 0);
        }
        else if (moveToDefaultWindowPos)
        {
            SetWindowPos(hWnd, HWND_TOPMOST, defaultWindowPos.left, defaultWindowPos.top, defaultWindowPos.right - defaultWindowPos.left, defaultWindowPos.bottom - defaultWindowPos.top, 0);
            moveToDefaultWindowPos = false;
        }

        SetLayeredWindowAttributes(hWnd, RGB(255, 255, 255), 128, LWA_ALPHA);
        SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOPMOST);
        return DefWindowProc(hWnd, message, wParam, lParam); 

    case WM_KILLFOCUS:
        SetLayeredWindowAttributes(hWnd, RGB(255, 255, 255), 0, LWA_ALPHA);
        SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT);
        return DefWindowProc(hWnd, message, wParam, lParam);

    case WM_SIZE:
    case WM_MOVE:
    {
        LRESULT result = DefWindowProc(hWnd, message, wParam, lParam);

        if (GetForegroundWindow() == hWnd)
        {
            int prevBottom = defaultWindowPos.bottom;
            GetWindowRect(hWnd, &defaultWindowPos);
            if (focusMode) defaultWindowPos.bottom = prevBottom;
        }

        return result;
    }

    case WM_TIMER:
        switch (wParam)
        {
        case IDT_REDRAW:
            if (focusMode)
            {
                HWND hwndForeground = GetForegroundWindow();
                if (hwndForeground != hWnd)
                {
                    // Add a slight delay between activating a window and moving the PoS window. 
                    // This makes changing the focused window smoother, less jumpy.
                    if (newFocusHwnd != hwndForeground)
                    {
                        newFocusHwnd = hwndForeground;
                        focusTimeMs = 0;
                    }
                    else
                        focusTimeMs += REDRAW_INTERVAL_MS;

                    if (focusTimeMs < FOCUS_SWITCH_DELAY_MS)
                        break;

#ifdef AUTO_REMOVE_CAPTION
                    // Remove PoS caption after one minute. Can't do it earlier because you can't select WS_POPUP windows when sharing a window.
                    if (focusTimeMs > 60000)
                        SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE | WS_DISABLED | WS_POPUP);
#endif

                    RECT rectForeground;
                    GetWindowRect(hwndForeground, &rectForeground);

                    RECT rectPoS;
                    GetWindowRect(hWnd, &rectPoS);

                    if (rectPoS.left != rectForeground.left ||
                        rectPoS.top != rectForeground.top ||
                        rectPoS.right != rectForeground.right ||
                        rectPoS.bottom != rectForeground.bottom)
                    {
                        SetWindowPos(hWnd, HWND_TOPMOST, rectForeground.left, rectForeground.top, rectForeground.right - rectForeground.left, rectForeground.bottom - rectForeground.top, SWP_NOACTIVATE);
                    }
                }
            }
            
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rectClient;
        GetClientRect(hWnd, &rectClient);

        HDC hdcDesktop = GetDC(NULL);
        POINT clientPoint = { 0, 0 };
        ClientToScreen(hWnd, &clientPoint);

        BitBlt(
            hdc,
            0,
            0,
            rectClient.right - rectClient.left,
            rectClient.bottom - rectClient.top,
            hdcDesktop,
            clientPoint.x,
            clientPoint.y,
            SRCCOPY | CAPTUREBLT);

        ReleaseDC(NULL, hdcDesktop);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* lpMMI = (MINMAXINFO*)lParam;
        lpMMI->ptMinTrackSize.x = POS_MIN_WIDTH;
        if (focusMode)
        {
            lpMMI->ptMinTrackSize.y = GetSystemMetrics(SM_CYCAPTION);
            // Prevent vertical sizing if the PoS window has the focus
            if (hWnd == GetForegroundWindow())
                lpMMI->ptMaxTrackSize.y = lpMMI->ptMinTrackSize.y;
        }
        else
            lpMMI->ptMinTrackSize.y = POS_MIN_HEIGHT;
    }
    break;

    case WM_DESTROY:
        SaveSettings();
        PostQuitMessage(0);
        break;

    case WM_SYSCOMMAND:
        if (wParam == IDC_OPTIONS)
        {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        }
        // fall through

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        SendMessage(GetDlgItem(hDlg, IDC_FOCUS_MODE), BM_SETCHECK, focusMode ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessage(GetDlgItem(hDlg, IDC_FIXED_MODE), BM_SETCHECK, !focusMode ? BST_CHECKED : BST_UNCHECKED, 0);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            if (!focusMode) moveToDefaultWindowPos = true;
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

        if (LOWORD(wParam) == IDC_FOCUS_MODE || LOWORD(wParam) == IDC_FIXED_MODE)
        {
            UINT checkState = (UINT) SendMessage(GetDlgItem(hDlg, IDC_FOCUS_MODE), BM_GETCHECK, 0, 0);
            focusMode = checkState == BST_CHECKED;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Returns the path to the settings INI file in %APPDATA%\PortionOfScreen\settings.ini
static BOOL GetSettingsPath(WCHAR* path, DWORD cchPath)
{
    WCHAR appData[MAX_PATH];
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appData)))
        return FALSE;

    StringCchPrintfW(path, cchPath, L"%s\\PortionOfScreen", appData);
    CreateDirectoryW(path, NULL);
    StringCchPrintfW(path, cchPath, L"%s\\PortionOfScreen\\settings.ini", appData);
    return TRUE;
}

void LoadSettings()
{
    WCHAR iniPath[MAX_PATH];
    if (!GetSettingsPath(iniPath, MAX_PATH) ||
        GetPrivateProfileIntW(L"Window", L"Left", -1, iniPath) == -1)
    {
        // No settings file or first run - use defaults
        defaultWindowPos.left = 100;
        defaultWindowPos.top = 100;
        defaultWindowPos.right = 900;
        defaultWindowPos.bottom = 700;
        focusMode = true;
        return;
    }

    defaultWindowPos.left   = GetPrivateProfileIntW(L"Window", L"Left",   100, iniPath);
    defaultWindowPos.top    = GetPrivateProfileIntW(L"Window", L"Top",    100, iniPath);
    defaultWindowPos.right  = GetPrivateProfileIntW(L"Window", L"Right",  900, iniPath);
    defaultWindowPos.bottom = GetPrivateProfileIntW(L"Window", L"Bottom", 700, iniPath);
    focusMode = GetPrivateProfileIntW(L"Settings", L"FocusMode", 1, iniPath) != 0;
}

void SaveSettings()
{
    WCHAR iniPath[MAX_PATH];
    if (!GetSettingsPath(iniPath, MAX_PATH))
        return;

    WCHAR buf[32];
    StringCchPrintfW(buf, 32, L"%d", defaultWindowPos.left);
    WritePrivateProfileStringW(L"Window", L"Left", buf, iniPath);
    StringCchPrintfW(buf, 32, L"%d", defaultWindowPos.top);
    WritePrivateProfileStringW(L"Window", L"Top", buf, iniPath);
    StringCchPrintfW(buf, 32, L"%d", defaultWindowPos.right);
    WritePrivateProfileStringW(L"Window", L"Right", buf, iniPath);
    StringCchPrintfW(buf, 32, L"%d", defaultWindowPos.bottom);
    WritePrivateProfileStringW(L"Window", L"Bottom", buf, iniPath);
    StringCchPrintfW(buf, 32, L"%d", focusMode ? 1 : 0);
    WritePrivateProfileStringW(L"Settings", L"FocusMode", buf, iniPath);
}
