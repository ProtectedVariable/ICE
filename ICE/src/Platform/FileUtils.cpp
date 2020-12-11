//
// Created by Thomas Ibanez on 09.12.20.
//

#include <Util/Logger.h>
#include "FileUtils.h"
#include "dialog.h"


namespace ICE {
    const std::string FileUtils::openFileDialog(const std::string& filter) {
        const std::string file = open_native_dialog(filter);
        Logger::Log(Logger::DEBUG, "Platform", "User selected file: %s", file.c_str());
        return file;
    }

    const std::string FileUtils::openFolderDialog() {
        const std::string folder = open_native_folder_dialog();
        Logger::Log(Logger::DEBUG, "Platform", "User selected folder: %s", folder.c_str());
        return folder;
    }
}
