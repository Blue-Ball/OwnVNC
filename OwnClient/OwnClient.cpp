// OwnClient.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "OwnClient.h"
#include <time.h>
#include <rfb/rfbclient.h>

#define FPS 25
#define UPDATE_INTERVAL (CLOCKS_PER_SEC/FPS) //(CLOCKS_PER_SEC / 10)

#define MAX_LOADSTRING 100

rfbClient*  g_client = NULL;
HWND        g_hWnd = NULL;

int OutputW(const WCHAR* format, ...)
{
	static WCHAR s_printf_buf[1024];
	va_list args;
	va_start(args, format);
	_vsnwprintf(s_printf_buf, sizeof(s_printf_buf), format, args);
	va_end(args);
	OutputDebugStringW(s_printf_buf);
    OutputDebugStringW(L"\n");
	return 0;
}

int OutputA(const char* format, ...)
{
	static char s_printf_buf[1024];
	va_list args;
	va_start(args, format);
	_vsnprintf(s_printf_buf, sizeof(s_printf_buf), format, args);
	va_end(args);
	OutputDebugStringA(s_printf_buf);
    OutputDebugStringA("\n");
	return 0;
}

static void PrintRect(rfbClient* client, int x, int y, int w, int h) {

	OutputA("%d - Received an update for %d,%d,%d,%d.", (int)time(0), x, y, w, h);
}

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    LPWSTR* argv;
    int     argc;

    argv = CommandLineToArgvW(lpCmdLine, &argc);

    if (argc != 4)
    {
        OutputW(L"%s", L"Invalid Arguments.");
        return 0;
    }

    char szHost[MAX_PATH];

    sprintf(szHost, "%ws", argv[0]);

    int     nPort = _wtoi(argv[1]);
    int     nWidth = _wtoi(argv[2]);
    int     nHeight = _wtoi(argv[3]);
    

    OutputW(L"Host: %S, Port: %d, Width: %d, Height: %d", szHost, nPort, nWidth, nHeight);
    
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	g_client = rfbGetClient(8, 3, 4);
    g_client->serverHost = szHost;
	g_client->serverPort = nPort;
	g_client->GotFrameBufferUpdate = PrintRect;

    try
    {
		if (!rfbInitClient(g_client, NULL, NULL))
		{
            MessageBox(NULL, L"FALSE", NULL, NULL);
            return 0;
		}
	}
	catch (...)
	{
        MessageBox(NULL, L"ERROR", NULL, NULL);
        return 0;
	}


    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_OWNCLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OWNCLIENT));

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

    KillTimer(g_hWnd, 0);
	if (g_client->frameBuffer)
		free(g_client->frameBuffer);
	// rfbClientCleanup(client);

    LocalFree(argv);
    msg.wParam = WM_QUIT;
    return (int) msg.wParam;
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

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OWNCLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName = NULL; // MAKEINTRESOURCEW(IDC_OWNCLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   g_hWnd = CreateWindowExW(0, szWindowClass, szTitle, WS_POPUP,
       200, 200, 500, 500, nullptr, nullptr, hInstance, nullptr);

   if (!g_hWnd)
   {
      return FALSE;
   }

   ShowWindow(g_hWnd, nCmdShow);
   UpdateWindow(g_hWnd);

   SetTimer(g_hWnd, 0, UPDATE_INTERVAL, NULL);
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
    int i;

    switch (message)
    {
    case WM_COMMAND:
        {
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
    case WM_TIMER:
		i = WaitForMessage(g_client, UPDATE_INTERVAL*1000);
		if (i < 0)
		{
            return 0;
		}
		if (i)
		{
			if (!HandleRFBServerMessage(g_client))
			{
                return 0;
			}
		}

        RECT        rt;
        GetClientRect(g_hWnd, &rt);
        InvalidateRect(g_hWnd, &rt, FALSE);
        break;
    case WM_NCHITTEST:
        return HTCAPTION;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
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
