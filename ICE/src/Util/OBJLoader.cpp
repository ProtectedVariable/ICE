//
// Created by Thomas Ibanez on 16.11.20.
//

#include "OBJLoader.h"

Mesh *ICE::OBJLoader::loadFromOBJ(std::string path) {
    bool loadOK = internalLoader.LoadFile(path);
    if(loadOK) {
        Mesh* mesh = new Mesh();
    }
    return nullptr;
}

ICE::OBJLoader::OBJLoader() : internalLoader(objl::Loader()) {};