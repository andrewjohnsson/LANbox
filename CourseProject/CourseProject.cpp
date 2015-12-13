// CourseProject.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "CourseProject.h"
#include "md5.h"
#include <vector>
#include <fstream>
#include <direct.h>
#include <commctrl.h>
#include <windows.h>
#include "winsock2.h"
#pragma comment (lib, "Ws2_32.lib")

using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE	hInst;														// current instance
HWND		mainWindow, loginWindow, usernameField, passwordField, 
			serverAddress, arrangeBox, fileListView;
LV_ITEM		LvItem;
LVCOLUMN	LvCol;
CHAR		directory[MAX_PATH];

TCHAR		szTitle[MAX_LOADSTRING];									// The title bar text
TCHAR		szWindowClass[MAX_LOADSTRING];								// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_COURSEPROJECT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) { return FALSE; }

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COURSEPROJECT));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
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
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_COURSEPROJECT));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+2);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_COURSEPROJECT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

LPWSTR CharToLPWSTR(LPCSTR char_string)
{
	DWORD res_len = MultiByteToWideChar(1251, 0, char_string, -1, NULL, 0);
	LPWSTR res = (LPWSTR)GlobalAlloc(GPTR, (res_len + 1) * sizeof(WCHAR));
	MultiByteToWideChar(1251, 0, char_string, -1, res, res_len);
	return res;
}

LPWSTR ConvertToLPWSTR(const std::string& s)
{
	LPWSTR ws = new wchar_t[s.size() + 1]; // +1 for zero at the end
	copy(s.begin(), s.end(), ws);
	ws[s.size()] = 0; // zero at the end
	return ws;
}

char* getHash(char* file){													//TO-DO: write hash calculator
	char* hash = "8dccad143277578943d380a89a954025";
	return hash;
}

void updateList() {
	ListView_DeleteAllItems(fileListView);
	ListView_Update(fileListView, NULL);
}

void setDirPath() {
	strcpy(directory, getenv("USERPROFILE"));
	strcat(directory, "\\Documents\\LanBox\\");

	if (_chdir(directory))
	{
		if (errno == ENOENT) {
			if (_mkdir(directory) == 0) { return; }
			else {
				MessageBox(NULL, L"Can't make initial dir! Try to launch with administrator rights.", L"Error", NULL);
				exit(NULL);
			}
		}
	}
	else { return; }
}

string fullPath(const char* file) {
	char path[320];
	strcpy(path, directory);
	strcat(path, file);
	return path;
}

vector<string> getFiles() {
	vector<string> vect;
	char path[500];
	char filename[320];
	strcpy(path, directory);
	strcat(path, "*.*");
	updateList();

	WIN32_FIND_DATA data;
	HANDLE hFile = FindFirstFileA(path, (LPWIN32_FIND_DATAA)&data);
	while (FindNextFile(hFile, &data) != 0)
	{
		if (!(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && !(data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && data.cFileName != L"." && data.cFileName != L"..")
		{
			size_t len = wcstombs(filename, data.cFileName, wcslen(data.cFileName));
			char* file = new char[len];
			filename[len] = '\0';
			if (!(string(filename) == string(".."))) {
				vect.push_back(string(filename));
			}
			LvItem.iItem = 1;
			LvItem.iSubItem = 0;
			LvItem.pszText = data.cFileName;
			SendMessage(fileListView, LVM_INSERTITEM, (WPARAM)-1, (LPARAM)&LvItem);
			LvItem.iSubItem = 1;
			LvItem.pszText = L"123";
			SendMessage(fileListView, LVM_SETITEM, 0, (LPARAM)&LvItem);
			LvItem.iSubItem = 2;
			LvItem.pszText = ConvertToLPWSTR(md5((string)file));
			SendMessage(fileListView, LVM_SETITEM, 0, (LPARAM)&LvItem);
		}
	}
	FindClose(hFile);
	return vect;
}

void startNetwork(char* ip, char* username, char* password)
{
	WSADATA WsaData;
	sockaddr_in peer;
	int count,sock;

	if (WSAStartup(0x0202, &WsaData) >= 0)
	{
		peer.sin_family = AF_INET;
		peer.sin_port = htons(8800);
		peer.sin_addr.s_addr = inet_addr(ip);
		sock = socket(AF_INET, SOCK_STREAM, 0);
		
		if (sock >= 0) {
			char buf[12], bufRecv[13], filesCount[3], filenameLength[3], fileLength[10];
			
			if (connect(sock, (sockaddr*)&peer, sizeof(peer)) != SOCKET_ERROR) {
				send(sock, username, sizeof(buf), 0);
				recv(sock, bufRecv, sizeof(bufRecv), 0);

				vector<string> files = getFiles();
				int filesVectorCount = files.size();
				if ((string)bufRecv == "OK") {
					send(sock, itoa(filesVectorCount, filesCount, 10), 3, 0);
					for (int i = 0; i < filesVectorCount; i++) {
						HANDLE FileOut;
						DWORD m;

						int strLen = files[i].length();
						send(sock, itoa(strLen, filenameLength, 10), 3, 0);
						send(sock, files[i].c_str(), strLen, 0);
						FileOut = CreateFile(CharToLPWSTR(directory), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
						count = GetFileSize(FileOut, NULL);
						char * bufs = new char[count];
						ReadFile(FileOut, bufs, count, &m, NULL);
						send(sock, itoa(count, fileLength, 10), 10, 0);
						send(sock, bufs, count, 0);
					}
					getFiles();
					ShowWindow(loginWindow, SW_HIDE);
					ShowWindow(mainWindow, SW_SHOW);
					UpdateWindow(mainWindow);
				}
				else { MessageBox(NULL, L"Wrong Credentials! Please Re-Check.", L"Error", NULL); }
			}
			else {
				MessageBox(NULL, L"Can't Connect!", L"Error", NULL);
			}

			closesocket(sock);
		}
		else { MessageBox(NULL, L"Can't create socket!", L"Error", NULL); }
	}
	else { MessageBox(NULL, L"Can't start WSA!", L"Error", NULL); }
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
	setDirPath();

	hInst = hInstance; // Store instance handle in our global variable

	loginWindow = CreateWindow(szWindowClass, L"Welcome to LanBox", WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX,
		(GetSystemMetrics(SM_CXSCREEN)-330)/2, (GetSystemMetrics(SM_CYSCREEN) - 185) / 2, 
		330, 185, NULL, NULL, hInstance, NULL);

	usernameField = CreateWindow(WC_EDIT, L"Username", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
		5, 10, 305, 20, loginWindow, (HMENU)IDC_START_USERNAME, hInstance, NULL);

	passwordField = CreateWindow(WC_EDIT , L"password", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
		5, 35, 305, 20, loginWindow, (HMENU)IDC_START_PASSWORD, hInstance, NULL);
	
	serverAddress = CreateWindow(WC_EDIT, L"127.0.0.1", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
		5, 60, 305, 20, loginWindow, (HMENU)IDC_START_IP, hInstance, NULL);

	HWND loginBtn = CreateWindow(WC_BUTTON, L"Sign In", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		125, 95, 65, 25, loginWindow, (HMENU)IDC_START_SIGNIN, hInstance, NULL);

	mainWindow = CreateWindow(szWindowClass, L"LanBox - Connected", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_DLGFRAME,
		(GetSystemMetrics(SM_CXSCREEN) - 640) / 2, (GetSystemMetrics(SM_CYSCREEN) - 360) / 2, 
		640, 360, NULL, NULL, hInstance, NULL);

	arrangeBox = CreateWindow(WC_COMBOBOX, NULL,
		CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		510, 5, 110, 75, mainWindow, (HMENU)IDC_MAIN_COMBOBOX_ARRANGE, hInstance, NULL);

	SendMessage(arrangeBox, CB_ADDSTRING, 0, (LPARAM)_T("List"));
	SendMessage(arrangeBox, CB_ADDSTRING, 0, (LPARAM)_T("Icons"));
	SendMessage(arrangeBox,CB_SETCURSEL, 0, 0);

	HWND syncBtn = CreateWindow(WC_BUTTON, L"Sync", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		510, 270, 110, 25, mainWindow, (HMENU)IDC_MAIN_BTN_SYNC, hInstance, NULL);

	HWND refreshBtn = CreateWindow(WC_BUTTON, L"Refresh", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		510, 240, 110, 25, mainWindow, (HMENU)IDC_MAIN_BTN_REFRESH, hInstance, NULL);

	fileListView = CreateWindow(WC_LISTVIEW, L"", WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE,
		5, 5, 500, 290, mainWindow, (HMENU)IDC_MAIN_FILELIST, hInstance, NULL);

	LvItem.mask = LVIF_TEXT;
	LvItem.cchTextMax = 256;
	
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	LvCol.pszText = L"Name";
	LvCol.cx = 240;
	SendMessage(fileListView, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol);

	LvCol.pszText = L"Type";
	LvCol.cx = 50;
	SendMessage(fileListView, LVM_INSERTCOLUMN, 1, (LPARAM)&LvCol);
	
	LVCOLUMN lvHashCol = LvCol;
	lvHashCol.pszText = L"Hashsum";
	lvHashCol.cx = 210;
	SendMessage(fileListView, LVM_INSERTCOLUMN, 2, (LPARAM)&lvHashCol);

	if (!loginWindow) { return FALSE; }

	ShowWindow(loginWindow, nCmdShow);
	UpdateWindow(loginWindow);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDC_START_SIGNIN:
			TCHAR username[32],password[32],ip[32];
			GetWindowText(usernameField, username, 32);
			GetWindowText(passwordField, password, 32);
			GetWindowText(serverAddress, ip, 32);
			//ip, username, password);
			startNetwork("127.0.0.1", "ada", "ih8winapi");
			break;
		case IDC_MAIN_BTN_REFRESH:
			updateList();
			break;
		case IDC_MAIN_COMBOBOX_ARRANGE:
			switch (HIWORD(wParam))
			{
				case CBN_SELCHANGE:
				if (int selRow = SendMessage(arrangeBox, CB_GETCURSEL, 0, 0) == 0){
					SendMessage(arrangeBox, LVM_SETVIEW, LV_VIEW_DETAILS, 0);
				}
				else {
					SendMessage(arrangeBox, LVM_SETVIEW, LV_VIEW_ICON, 0);
				}
				break;
			}
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

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