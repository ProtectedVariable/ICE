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

const std::string open_native_dialog(std::string const& filter) {

	char filename[MAX_PATH];

	OPENFILENAME ofn;
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	//"Filter\0*.PDF\0";
	std::string mfilter("Filter\0*.", 8);
	ofn.lpstrFilter = (mfilter + filter + std::string("\0\0\0", 2)).c_str();
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = _T("Select a file");
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
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