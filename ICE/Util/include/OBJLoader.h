//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_OBJLOADER_H
#define ICE_OBJLOADER_H
#include <Mesh.h>
#include <OBJLoader/tiny_obj_loader.h>

#include <memory>

namespace ICE {
class OBJLoader {
   public:
    static std::shared_ptr<Mesh> loadFromOBJ(const std::string& path);
};
}  // namespace ICE

#endif  //ICE_OBJLOADER_H
