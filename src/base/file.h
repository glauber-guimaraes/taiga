/*
** Taiga
** Copyright (C) 2010-2014, Eren Okka
** 
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TAIGA_BASE_FILE_H
#define TAIGA_BASE_FILE_H

#include <string>
#include <vector>
#include <windows.h>

#include "types.h"

unsigned long GetFileAge(const std::wstring& path);
QWORD GetFileSize(const std::wstring& path);
QWORD GetFolderSize(const std::wstring& path, bool recursive);

bool Execute(const std::wstring& path, const std::wstring& parameters = L"");
BOOL ExecuteEx(const std::wstring& path, const std::wstring& parameters = L"");
void ExecuteLink(const std::wstring& link);

std::wstring BrowseForFile(HWND hwndOwner, LPCWSTR lpstrTitle, LPCWSTR lpstrFilter = nullptr);
BOOL BrowseForFolder(HWND hwnd, const std::wstring& title, const std::wstring& default_path, std::wstring& output);

bool CreateFolder(const std::wstring& path);
int DeleteFolder(std::wstring path);

bool FileExists(const std::wstring& file);
bool FolderExists(const std::wstring& folder);
bool PathExists(const std::wstring& path);
void ValidateFileName(std::wstring& path);

std::wstring ExpandEnvironmentStrings(const std::wstring& path);
std::wstring GetDefaultAppPath(const std::wstring& extension, const std::wstring& default_value);

unsigned int PopulateFiles(std::vector<std::wstring>& file_list, const std::wstring& path, const std::wstring& extension = L"", bool recursive = false, bool trim_extension = false);
int PopulateFolders(std::vector<std::wstring>& folder_list, const std::wstring& path);

bool ReadFromFile(const std::wstring& path, std::string& output);
bool SaveToFile(LPCVOID data, DWORD length, const std::wstring& path, bool take_backup = false);

std::wstring ToSizeString(QWORD qwSize);

#endif  // TAIGA_BASE_FILE_H