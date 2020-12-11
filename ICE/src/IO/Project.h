//
// Created by Thomas Ibanez on 09.12.20.
//

#ifndef ICE_PROJECT_H
#define ICE_PROJECT_H

#include <string>

namespace ICE {
    class Project {
    public:
        bool CreateDirectories();

        Project(const std::string &baseDirectory, const std::string &name);

        const std::string &getBaseDirectory() const;

        const std::string &getName() const;

    private:
        std::string baseDirectory;
        std::string name;
    };
}


#endif //ICE_PROJECT_H
