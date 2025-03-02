//
// Created by Thomas Ibanez on 09.12.20.
//

#ifndef ICE_FILEUTILS_H
#define ICE_FILEUTILS_H


#include <string>

namespace ICE {
    class FileUtils {

    public:
        static const std::string openFileDialog(const std::string& filter);
        static const std::string openFolderDialog();
        static const std::string readFile(const std::string &path);
    };
}


#endif //ICE_FILEUTILS_H
