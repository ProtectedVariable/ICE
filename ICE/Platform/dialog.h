//
// Created by Thomas Ibanez on 09.12.20.
//

#ifndef ICE_PLATEFORM_DIALOG_H
#define ICE_PLATEFORM_DIALOG_H

#include <string>
#include <vector>

struct FileFilter {
    std::string description;
    std::string extension;
};

const std::string open_native_dialog(const std::vector<FileFilter>& filters);
const std::string open_native_folder_dialog();

#endif