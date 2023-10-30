// AppsflyerSampleApp.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "AppsflyerSampleApp.h"
#include "AppsflyerLauncherModule.h"

#define MAX_LOADSTRING 100
#define BTN_LOG_EVENT 1001
#define BTN_SDK_STOP 1002

// Global Variables:
HINSTANCE hInst;                     // current instance
WCHAR szTitle[MAX_LOADSTRING];       // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_APPSFLYERSAMPLEAPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_APPSFLYERSAMPLEAPP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPSFLYERSAMPLEAPP));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_APPSFLYERSAMPLEAPP);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    HWND hButton = CreateWindow("button", "InApp Event (LogEvent() function)",
                                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                100, 200,
                                300, 30,
                                hWnd, (HMENU)BTN_LOG_EVENT,
                                hInst, NULL);
    
    HWND hButton2 = CreateWindow("button", "Stop the SDK [LogEvent will stop working]",
                                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                100, 300,
                                300, 60,
                                hWnd, (HMENU)BTN_SDK_STOP,
                                hInst, NULL);
    // Initialize the AF connector
    //AppsflyerLauncherModule()->Init(<< DEV_KEY >> , << APP_ID >>);
    AppsflyerLauncherModule()->Init("bFzaVu2iecN77po5mWMJuL", "12341234");
    //AppsflyerLauncherModule()->SetCustomerUserId("cuid-test");
    AppsflyerLauncherModule()->Start();

    // will return false
    bool dateBefore = AppsflyerLauncherModule()->IsInstallOlderThanDate("2023-January-01 23:12:34");

    // will return true
    bool dateAfter = AppsflyerLauncherModule()->IsInstallOlderThanDate("2024-April-10 23:12:34");

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        json event_parameters = {};
        json event_custom_parameters = {};
        std::string event_name = "af_purchase";
        std::wstring ws = L"車B1234 こんにちは";
        std::wstring ws2 = L"新人邀约购物日";

        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case BTN_LOG_EVENT:
            // Setting the event values json and event name
            event_parameters = { 
                {"af_currency", "USD"}, 
                {"af_price", 6.66}, 
                {"af_revenue", 24.12}
            };
            event_custom_parameters = { 
                {"goodsName", AppsflyerLauncherModule()->to_utf8(ws)}, 
                {"goodsName2", AppsflyerLauncherModule()->to_utf8(ws2)} 
            };
            event_name = "af_purchase";
            // Send LogEvent request
            AppsflyerLauncherModule()->LogEvent(event_name, event_parameters);
            // Send LogEvent request with custom event params
            AppsflyerLauncherModule()->LogEvent(event_name, event_parameters, event_custom_parameters);
            break;
        case BTN_SDK_STOP:
            AppsflyerLauncherModule()->Stop();
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
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
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
