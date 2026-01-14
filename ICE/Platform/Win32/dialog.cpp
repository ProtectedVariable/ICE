#include "dialog.h"
#include <string>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <iostream>
#include <tchar.h>
#include <shobjidl.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <combaseapi.h>

const std::string open_native_dialog(const std::vector<FileFilter>& filters)
{
    char filename[MAX_PATH] = { 0 };

    // Build filter string: "Description1 (*.ext1;*.ext2)\0*.ext1;*.ext2\0Description2 (*.ext3)\0*.ext3\0\0"
    std::string filterStr;
    for (const auto& filter : filters)
    {
        // Show both description and extension pattern in the filter name
        filterStr += filter.description;
        filterStr += " (";
        filterStr += filter.extension;
        filterStr += ")";
        filterStr += '\0';
        filterStr += filter.extension;
        filterStr += '\0';
    }
    filterStr += '\0'; // Double null-terminated

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = filters.empty() ? NULL : filterStr.c_str();
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Select a file";
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn))
    {
        // Ensure null-termination
        filename[MAX_PATH - 1] = '\0';
        return std::string(filename);
    }
    return std::string();
}

const std::string open_native_folder_dialog() {

	DWORD dwOptions = 0;
	std::string str = "";
	// Create dialog
	::IFileOpenDialog* fileDialog(NULL);
	HRESULT result = CoCreateInstance(CLSID_FileOpenDialog,
		NULL,
		CLSCTX_ALL,
		IID_PPV_ARGS(&fileDialog));
	if (!SUCCEEDED(result))
	{
		goto end;
	}
	// Get the dialogs options
	if (!SUCCEEDED(fileDialog->GetOptions(&dwOptions)))
	{
		goto end;
	}
	// Add in FOS_PICKFOLDERS which hides files and only allows selection of folders
	if (!SUCCEEDED(fileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS)))
	{
		goto end;
	}
	// Show the dialog to the user
	result = fileDialog->Show(NULL);
	if (SUCCEEDED(result))
	{
		// Get the folder name
		::IShellItem* shellItem(NULL);
		result = fileDialog->GetResult(&shellItem);
		if (!SUCCEEDED(result))
		{
			shellItem->Release();
			goto end;
		}
		wchar_t* path = NULL;
		result = shellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &path);
		if (!SUCCEEDED(result))
		{
			shellItem->Release();
			goto end;
		}
		std::wstring ws(path);
		str = std::string(ws.begin(), ws.end());
		shellItem->Release();
	}
end:
	if (fileDialog)
		fileDialog->Release();
	return str;
}