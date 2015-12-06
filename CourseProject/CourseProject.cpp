#include "stdafx.h"
#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "Resource.h"
#include <commctrl.h>
#include <wincrypt.h>
#include <fstream>
#include <shlobj.h>

using namespace std;

typedef struct File
{
	char name[255];
	bool isFolder;
	char location[600];
	char hash[32];
};

File filelist;
LV_ITEM LvItem;
LVCOLUMN LvCol;
char buffer[500];

LPWSTR CharToLPWSTR(LPCSTR char_string);

HWND hwndList;
CHAR directory[MAX_PATH];
HRESULT result = SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, CharToLPWSTR(directory));

INT_PTR CALLBACK Processor(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

LPWSTR CharToLPWSTR(LPCSTR char_string)
{
	LPWSTR res;
	DWORD res_len = MultiByteToWideChar(1251, 0, char_string, -1, NULL, 0);
	res = (LPWSTR)GlobalAlloc(GPTR, (res_len + 1) * sizeof(WCHAR));
	MultiByteToWideChar(1251, 0, char_string, -1, res, res_len);
	return res;
}

void MD5Hash(BYTE hash[], int sz, char sec[]) {
	HCRYPTPROV hProv = 0, hHash = 0;
	BYTE rgbHash[16];
	DWORD cbHash = 0;
	char file[MAX_PATH], dig[] = "0123456789abcdef";
	int l = 0;

	CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash);
	CryptHashData(hHash, hash, sz, 0);
	cbHash = 16;
	CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0);

	for (DWORD i = 0; i<cbHash; i++) {
		sec[l] = dig[rgbHash[i] >> 4];
		l++;
		sec[l] = dig[rgbHash[i] & 0xf];
		l++;
	}
	for (l = 32; l<strlen(sec); l++)sec[l] = 0;

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
}

LPWSTR MessageBoxFileHashSum(char buffer[]) {
	char file[600];
	strcpy(file, directory);
	strcat(file, buffer);
	ifstream read(file, ios::binary);
	read.seekg(0, ios::end);
	int l = read.tellg();
	read.seekg(0, ios::beg);
	char * buf = 0;
	if ((buf = (char*)malloc(l)) == NULL) {
		MessageBox(0, CharToLPWSTR("Directorys cannot be summed"), 0, 0);
		return 0;
	}
	read.read(buf, l);
	if (read.fail()) {
		free(buf);
		MessageBox(NULL, CharToLPWSTR("Error Opening File"), CharToLPWSTR("andrewjohnsson"), MB_OK);
		return 0;
	}
	read.close();

	char hash[32];
	MD5Hash((BYTE*)buf, l, hash);
	free(buf);
	return CharToLPWSTR(hash);
	//MessageBox(NULL, CharToLPWSTR(hash), CharToLPWSTR("File Checksum:"), MB_OK);
}

void UpdateList() {
	char path[500];
	int i = 0;
	strcpy(path, directory);
	strcat(path, "*.*");

	WIN32_FIND_DATA data;
	HANDLE hFile = FindFirstFileA(path, (LPWIN32_FIND_DATAA)&data);
	SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
	while (FindNextFile(hFile, &data) != 0)
	{
		LvItem.iItem = (WPARAM)-1;
		LvItem.iSubItem = 0;
		LvItem.pszText = data.cFileName;
		SendMessage(hwndList, LVM_INSERTITEM, (WPARAM)-1, (LPARAM)&LvItem);
		LvItem.iItem = i;
		LvItem.pszText = MessageBoxFileHashSum(buffer);
		LvItem.iSubItem = 2;
		SendMessage(hwndList, LVM_SETITEM, 0, (LPARAM)&LvItem);
	}
	FindClose(hFile);
}

int CALLBACK _tWinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE hPrevInstance,
	_In_  LPTSTR lpCmdLine,
	_In_  int nCmdShow)
{
	::DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, Processor);
	return 0;
}

INT_PTR CALLBACK Processor(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		hwndList = GetDlgItem(hDlg, IDC_LIST_FILES);
		
		LvItem.mask = LVIF_TEXT;
		LvItem.cchTextMax = 256;
		LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		LvCol.cx = 0x28;
		LvCol.pszText = CharToLPWSTR("Name");
		LvCol.cx = 0x70;
		SendMessage(hwndList, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol);
		LvCol.pszText = CharToLPWSTR("Location");
		SendMessage(hwndList, LVM_INSERTCOLUMN, 1, (LPARAM)&LvCol);
		LvCol.pszText = CharToLPWSTR("Hashsum");
		SendMessage(hwndList, LVM_INSERTCOLUMN, 2, (LPARAM)&LvCol);
		UpdateList();
		/*
		for (int i = 0; i < ARRAYSIZE(fileList); i++)
		{
			LvItem.iItem = i;
			LvItem.iSubItem = 0;
			LvItem.pszText = CharToLPWSTR(fileList[i].name);
			SendMessage(hwndList, LVM_INSERTITEM, 0, (LPARAM)&LvItem);
		}

		for (int i = 0; i < ARRAYSIZE(fileList); i++)
		{
			LvItem.iItem = i;
			LvItem.pszText = CharToLPWSTR(fileList[i].location);
			LvItem.iSubItem = 1;
			SendMessage(hwndList, LVM_SETITEM, 0, (LPARAM)&LvItem);
		}

		for (int i = 0; i < ARRAYSIZE(fileList); i++)
		{
			LvItem.iItem = i;
			LvItem.pszText = CharToLPWSTR(fileList[i].hash);
			LvItem.iSubItem = 2;
			SendMessage(hwndList, LVM_SETITEM, 0, (LPARAM)&LvItem);
		}*/

		SetFocus(hwndList);
		return TRUE;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDC_BUTTON_CONNECT:
				MessageBox(0, CharToLPWSTR("Connecting to"), CharToLPWSTR("Info"), MB_OK);
			case IDC_LIST_FILES:
			{
				switch (HIWORD(wParam))
				{
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}