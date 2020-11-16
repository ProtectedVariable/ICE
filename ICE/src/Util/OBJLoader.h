//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_OBJLOADER_H
#define ICE_OBJLOADER_H

#include <OBJLoader/OBJLoader.h>
#include <Graphics/Mesh.h>

namespace ICE {
    class OBJLoader {
    public:
        Mesh* loadFromOBJ(std::string path);
        OBJLoader();

    private:
        objl::Loader internalLoader;
    };
}


#endif //ICE_OBJLOADER_H
