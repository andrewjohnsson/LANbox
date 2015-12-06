#pragma once

#include "resource.h"
#include "vector"
using namespace std;

LPWSTR fromChar(const char* name) {
	wchar_t wtext[70];
	mbstowcs(wtext, name, strlen(name) + 1);//Plus null
	return wtext;
}

int GetTextSize(LPCWSTR a0)
{
	for (int iLoopCounter = 0; ; iLoopCounter++)
	{
		if (a0[iLoopCounter] == '\0')
			return iLoopCounter;
	}
}

bool isDirectoryExists(const char *filename)
{
	LPWSTR ptr = fromChar(filename);
	DWORD dwFileAttributes = GetFileAttributes(ptr);
	if (dwFileAttributes == 0xFFFFFFFF)
		return false;
	return dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}



vector<WCHAR*> get_all_files_names_within_folder(string folder)
{
	vector<WCHAR*> names;
	char search_path[200];
	sprintf(search_path, "%s/*.*", folder.c_str());
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(fromChar(search_path), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}