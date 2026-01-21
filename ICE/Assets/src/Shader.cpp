#include "Shader.h"

namespace ICE {

std::string Shader::getTypeName() const {
    return "Shader";
};

AssetType Shader::getType() const {
    return AssetType::EShader;
};

}  // namespace ICE
