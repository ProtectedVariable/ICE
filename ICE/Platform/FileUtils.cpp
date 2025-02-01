//
// Created by Thomas Ibanez on 09.12.20.
//

#include "FileUtils.h"

#include <Logger.h>

#include <fstream>

#include "dialog.h"

namespace ICE {
const std::string FileUtils::openFileDialog(const std::string &filter) {
    const std::string file = open_native_dialog(filter);
    Logger::Log(Logger::DEBUG, "Platform", "User selected file: %s", file.c_str());
    return file;
}

const std::string FileUtils::openFolderDialog() {
    const std::string folder = open_native_folder_dialog();
    Logger::Log(Logger::DEBUG, "Platform", "User selected folder: %s", folder.c_str());
    return folder;
}

const std::string FileUtils::readFile(const std::string &path) {
    std::ifstream ifs(path);
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    return content;
}
}  // namespace ICE
