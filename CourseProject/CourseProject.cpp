// CourseProject.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "CourseProject.h"
#include "iostream"
#define MAX_LOADSTRING 100
#define BUTTON_ID      1001

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

LPCWSTR TextArray[] = {
	L"Hello World"
};


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_COURSEPROJECT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COURSEPROJECT));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COURSEPROJECT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_COURSEPROJECT);
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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

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
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

HWND hBtn
, hLabel
, hListbox
, hTextBox
, hButton;

void InitializeComponent(HWND hWnd) {
	HINSTANCE hInstance = GetModuleHandle(NULL);

	// Adding a ListBox.
	hListbox = CreateWindowExW(WS_EX_CLIENTEDGE
		, L"LISTBOX", NULL
		, WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL
		, 7, 35, 300, 200
		, hWnd, NULL, hInstance, NULL);
	hButton = CreateWindow(L"button", L"Sync",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		100, 10,
		150, 30,
		hWnd, (HMENU)BUTTON_ID,
		hInst, NULL);
	SetWindowTextW(hTextBox, L"Input text here...");
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	vector<WCHAR*> files = get_all_files_names_within_folder("C:\\Ruby22-x64\\lib");
	int count = 0;
    switch (message)
    {
	case WM_CREATE:
			InitializeComponent(hWnd);
			SendMessage(hListbox, LB_ADDSTRING, 0, (LPARAM)L"name");
			SendMessage(hListbox, LB_ADDSTRING, 0, (LPARAM)L"extension");
			SendMessage(hListbox, LB_ADDSTRING, 0, (LPARAM)L"date");
			SendMessage(hListbox, LB_ADDSTRING, 0, (LPARAM)L"size");
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {

			case BUTTON_ID:
				MessageBox(NULL, L"Works", NULL, NULL);
				SendMessage(hListbox, LB_RESETCONTENT, 0, 0);

				break;
            case IDM_ABOUT:
				
				
				for (int i = 0; i < files.size();i++) {
					count++;
				}
				wchar_t buffer[256];
				wsprintfW(buffer, L"%d",count);
				MessageBoxW(nullptr, buffer, buffer, MB_OK);
				//MessageBox(NULL, L"I am just trying my wedding dress", NULL, NULL);
				if (isDirectoryExists("C:\\Users")) {
					MessageBox(NULL, L"I am just trying my wedding dress", NULL, NULL);
				}
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
			LPCWSTR d = L"sd";
			LPCWSTR f = L"f";
			std::wstring df = std::wstring(d) + f;
			LPCWSTR dfc = df.c_str(); // if you are really need this
			LPCWSTR myFiles = L"";
			for (int i = 0; i < (sizeof(TextArray) / sizeof(*TextArray));i++) {

			}
			TextOut(hdc,
				// Location of the text
				10,
				10,
				// Text to print
				TextArray[0],
				// Size of the text, my function gets this for us
				GetTextSize(TextArray[0]));
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
