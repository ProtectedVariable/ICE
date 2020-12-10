#include <Platform/dialog.h>
#include <string>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <iostream>
#include <tchar.h>

const std::string open_native_dialog(std::string const& filter) {

	char filename[MAX_PATH];

	OPENFILENAME ofn;
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	//"Filter\0*.PDF\0";
	std::string mfilter("Filter\0*.", 8);
	ofn.lpstrFilter = (mfilter+filter+std::string("\0\0\0",2)).c_str();
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