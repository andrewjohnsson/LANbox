#include "stdafx.h"
#include "CourseProject.h"

using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			hInst;														// current instance
HIMAGELIST			hImageList = ImageList_Create(16, 16, ILC_COLOR16, 1, 10);
HWND				mainWindow, loginWindow, usernameField, passwordField,
serverAddress, arrangeBox, fileListView;
int					sock;														// Socket
vector<string>		localFiles, remoteFiles, localNames, remoteNames, hashes, status;
LV_ITEM				LvItem;
LVCOLUMN			LvCol;
CHAR				directory[MAX_PATH];

TCHAR				szTitle[MAX_LOADSTRING];									// The title bar text
TCHAR				szWindowClass[MAX_LOADSTRING];								// the main window class name

																				// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void				addItem(int i);
bool				syncFiles();
bool				getLocalFiles();
bool				sendLocalNames();
bool				sendLocalFiles();
bool				getRemoteFiles();
bool				updateList();
void				setSynced();
bool				startNetwork(TCHAR* ip, TCHAR* username);
void				killNetwork();

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_COURSEPROJECT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	INITCOMMONCONTROLSEX icc;
	icc.dwICC = ICC_ANIMATE_CLASS | ICC_NATIVEFNTCTL_CLASS | ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
	icc.dwSize = sizeof(icc);
	InitCommonControlsEx(&icc);

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
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
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
	LPWSTR ws = new wchar_t[s.size() + 1];
	copy(s.begin(), s.end(), ws);
	ws[s.size()] = 0;
	return ws;
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

string fullPath(string file) {
	string s = string(directory) + file;
	return s;
}

void addItem(int i) {
	LvItem.iItem = i;
	LvItem.iSubItem = 0;
	LvItem.pszText = ConvertToLPWSTR(localNames[i]);
	ListView_InsertItem(fileListView, &LvItem);
	ListView_SetItemText(fileListView, 0, 1, ConvertToLPWSTR(status[i]));
	ListView_SetItemText(fileListView, 0, 2, ConvertToLPWSTR(hashes[i]));
}

bool getLocalFiles() {
	char	path[500], filename[256];
	int		i = 0;

	strcpy(path, directory);
	strcat(path, "*.*");

	WIN32_FIND_DATA		data;
	HANDLE				hFile = FindFirstFileA(path, (LPWIN32_FIND_DATAA)&data);
	while (FindNextFile(hFile, &data) != 0)
	{
		if ((data.dwFileAttributes > 16) && !(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && !(data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
		{
			if (string(filename) != string("..")) {
				HANDLE	FileOut;
				DWORD	m;
				bool	isRead = false;
				char*	bufs;
				int		count = 0;
				string	hash;

				size_t	len = wcstombs(filename, data.cFileName, wcslen(data.cFileName));

				filename[len] = '\0';
				localNames.push_back(string(filename));
				status.push_back("Local");
				FileOut = CreateFile(CharToLPWSTR(fullPath(localNames[i]).c_str()), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
				count = GetFileSize(FileOut, NULL);
				bufs = new char[count];

				while (isRead == FALSE) {
					if (ReadFile(FileOut, bufs, count, &m, NULL)) {
						isRead = true;
						break;
					}
					break;
				}

				hash = md5(string(bufs));
				hashes.push_back(hash);
				addItem(i);
				delete[] bufs;
				i++;
			}
		}
	}
	FindClose(hFile);
	return true;
}

bool sendLocalNames() {
	char	filesCount[3], filenameLength[3], fileLength[10];							// TO-DO: fix transmit bug

	send(sock, itoa(localNames.size(), filesCount, 10), 3, 0);

	for (int i = 0; i < localNames.size(); i++) {
		int	strLen = localNames[i].length();
		send(sock, itoa(strLen, filenameLength, 10), 3, 0);
		send(sock, localNames[i].c_str(), strLen, 0);
		send(sock, hashes[i].c_str(), 32, 0);
	}
	return true;
}

bool getRemoteFiles() {
	char	remoteFilesCount[3];

	recv(sock, remoteFilesCount, 3, 0);
	int remoteFilesCounter = atoi(remoteFilesCount);

	for (int i = 0; i < remoteFilesCounter; i++) {
		char fileNameLengthString[3];
		int fileNameLength;
		recv(sock, fileNameLengthString, 3, 0);
		fileNameLength = atoi(fileNameLengthString);

		char* fileName = new char[fileNameLength + 1];
		fileName[fileNameLength] = '\0';
		recv(sock, fileName, fileNameLength, 0);
		remoteNames.push_back(string(fileName));
	}

	for (int i = 0; i < remoteNames.size(); i++) {
		HANDLE	FileOut;
		DWORD	m;
		bool	isWritten = false;

		char fileLengthString[10];
		memset(fileLengthString, '\0', 10);

		FileOut = CreateFile(CharToLPWSTR(fullPath(remoteNames[i].c_str()).c_str()), GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);

		recv(sock, fileLengthString, 10, 0);
		char * bufs = new char[atoi(fileLengthString)];
		recv(sock, bufs, atoi(fileLengthString), 0);

		while (isWritten == FALSE) {
			if (WriteFile(FileOut, bufs, atoi(fileLengthString), &m, 0)) {
				isWritten = true;
				break;
			}
		}
		send(sock, "OK", 2, 0);
	}
	return true;
}

bool sendLocalFiles() {
	for (int i = 0; i < localNames.size(); i++) {
		HANDLE	FileOut;
		DWORD	m;
		bool	isRead = false;

		FileOut = CreateFile(CharToLPWSTR(fullPath(localNames[i].c_str()).c_str()), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		int count = GetFileSize(FileOut, NULL);
		char *file = new char[count];
		char fileLngth[10];
		memset(fileLngth, '\0', 10);

		send(sock, itoa(count, fileLngth, 10), 10, 0);
		while (isRead == FALSE) {
			if (ReadFile(FileOut, file, count, &m, NULL)) {
				isRead = true;
				break;
			}
			break;
		}
		send(sock, file, count, 0);
		char okGood[2];
		memset(okGood, '\0', 2);
		recv(sock, okGood, 2, 0);
		delete[] file;
	}
	killNetwork();
	return true;
}

bool syncFiles() {
	if (updateList()) {
		if (sendLocalNames()) {
			if (getRemoteFiles()) {
				if (sendLocalFiles()) {
					killNetwork();
					return true;
				}
			}
		}
	}
}

void setSynced() {
	for (int i = 0; i < localNames.size(); i++) {
		status[i] = "Synced";
		LvItem.iSubItem = 1;
		LvItem.pszText = ConvertToLPWSTR(status[i]);
		SendMessage(fileListView, LVM_SETITEM, 0, (LPARAM)&LvItem);
	}
}

bool updateList() {
	ListView_DeleteAllItems(fileListView);
	localNames.clear();
	localFiles.clear();
	remoteNames.clear();
	remoteFiles.clear();
	hashes.clear();
	getLocalFiles();
	return true;
}

bool startNetwork(TCHAR* ip, TCHAR* username)
{
	WSADATA	WsaData;

	char	address[32], user[32];					//Conversion of fields data
	wcstombs(address, ip, wcslen(ip) + 1);
	wcstombs(user, username, wcslen(username) + 1);

	if (WSAStartup(0x0202, &WsaData) >= 0)
	{
		sockaddr_in	peer;
		peer.sin_family = AF_INET;
		peer.sin_port = htons(8800);
		peer.sin_addr.s_addr = inet_addr(address);
		sock = socket(AF_INET, SOCK_STREAM, 0);

		if (sock >= 0) {
			char buf[12], bufRecv[16];

			if (connect(sock, (sockaddr*)&peer, sizeof(peer)) != SOCKET_ERROR) {
				send(sock, user, sizeof(buf), 0);
				recv(sock, bufRecv, 16, 0);

				if ((string)bufRecv == "OK") {
					syncFiles();
					setSynced();
					return true;
				}
				else { MessageBox(NULL, L"Wrong Credentials! Please Re-Check.", L"Error", NULL); }
			}
			else { MessageBox(NULL, L"Can't Connect!", L"Error", NULL); }
		}
		else { MessageBox(NULL, L"Can't create socket!", L"Error", NULL); }
	}
	else { MessageBox(NULL, L"Can't start WSA!", L"Error", NULL); }
	return false;
}

void killNetwork() {
	closesocket(sock);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	setDirPath();

	hInst = hInstance; // Store instance handle in our global variable

	loginWindow = CreateWindow(szWindowClass, L"Welcome to LanBox", WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX,
		(GetSystemMetrics(SM_CXSCREEN) - 330) / 2, (GetSystemMetrics(SM_CYSCREEN) - 185) / 2,
		330, 185, NULL, NULL, hInstance, NULL);

	usernameField = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, L"andrewjohnsson", WS_EX_CLIENTEDGE | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
		45, 10, 225, 25, loginWindow, (HMENU)IDC_START_USERNAME, hInstance, NULL);

	serverAddress = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, L"127.0.0.1", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
		45, 40, 225, 25, loginWindow, (HMENU)IDC_START_IP, hInstance, NULL);

	HWND loginBtn = CreateWindow(WC_BUTTON, L"Sign In", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		115, 90, 75, 25, loginWindow, (HMENU)IDC_START_SIGNIN, hInstance, NULL);

	mainWindow = CreateWindow(szWindowClass, L"LanBox - Connected", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_DLGFRAME,
		(GetSystemMetrics(SM_CXSCREEN) - 640) / 2, (GetSystemMetrics(SM_CYSCREEN) - 360) / 2,
		640, 360, NULL, NULL, hInstance, NULL);

	arrangeBox = CreateWindowEx(WS_EX_CLIENTEDGE, WC_COMBOBOX, NULL,
		CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		510, 5, 110, 75, mainWindow, (HMENU)IDC_MAIN_COMBOBOX_ARRANGE, hInstance, NULL);

	SendMessage(arrangeBox, CB_ADDSTRING, 0, (LPARAM)_T("List"));
	SendMessage(arrangeBox, CB_ADDSTRING, 0, (LPARAM)_T("Icons"));
	SendMessage(arrangeBox, CB_SETCURSEL, 0, 0);

	HWND syncBtn = CreateWindow(WC_BUTTON, L"Sync", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		510, 270, 110, 25, mainWindow, (HMENU)IDC_MAIN_BTN_SYNC, hInstance, NULL);

	HWND refreshBtn = CreateWindow(WC_BUTTON, L"Refresh", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		510, 240, 110, 25, mainWindow, (HMENU)IDC_MAIN_BTN_REFRESH, hInstance, NULL);

	fileListView = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_EX_FULLROWSELECT,
		5, 5, 500, 290, mainWindow, (HMENU)IDC_MAIN_FILELIST, hInstance, NULL);

	ListView_SetView(fileListView, LVS_REPORT);
	ListView_SetExtendedListViewStyle(fileListView, LVS_EX_FULLROWSELECT);

	LvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	LvItem.stateMask = 0;
	LvItem.iSubItem = 0;
	LvItem.state = 0;
	LvItem.cchTextMax = 256;

	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	LvCol.pszText = L"Name";
	LvCol.cx = 220;
	SendMessage(fileListView, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol);

	LvCol.pszText = L"Status";
	LvCol.cx = 50;
	SendMessage(fileListView, LVM_INSERTCOLUMN, 1, (LPARAM)&LvCol);

	LVCOLUMN lvHashCol = LvCol;
	lvHashCol.pszText = L"Hashsum";
	lvHashCol.cx = 220;
	SendMessage(fileListView, LVM_INSERTCOLUMN, 2, (LPARAM)&lvHashCol);

	if (!loginWindow) { return FALSE; }

	ShowWindow(loginWindow, nCmdShow);
	UpdateWindow(loginWindow);

	return TRUE;
}

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
			TCHAR username[32], ip[32];
			GetWindowText(usernameField, username, 32);
			GetWindowText(serverAddress, ip, 32);
			ShowWindow(loginWindow, SW_HIDE);
			ShowWindow(mainWindow, SW_SHOW);
			UpdateWindow(mainWindow);
			startNetwork(ip, username);
			break;
		case WM_CREATE:
			MessageBox(hWnd, L"Win Computer", L"Ok", MB_OK);
			break;
		case IDC_MAIN_BTN_REFRESH:
			ListView_DeleteAllItems(fileListView);
			getLocalFiles();
			break;
		case IDC_MAIN_BTN_SYNC:
			ListView_DeleteAllItems(fileListView);
			syncFiles();
			//setSynced();
			break;
		case IDC_MAIN_COMBOBOX_ARRANGE:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				if (int selRow = SendMessage(arrangeBox, CB_GETCURSEL, 0, 0) == 0) {
					ListView_SetView(fileListView, LVS_REPORT);
				}
				else {
					ListView_SetView(fileListView, LVS_ICON);
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
		killNetwork();
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