//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_OBJLOADER_H
#define ICE_OBJLOADER_H
#include <OBJLoader/tiny_obj_loader.h>
#include <Graphics/Mesh.h>

namespace ICE {
    class OBJLoader {
    public:
        static Mesh* loadFromOBJ(const std::string &path);
    };
}


#endif //ICE_OBJLOADER_H
