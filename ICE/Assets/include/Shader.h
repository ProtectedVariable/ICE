//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_SHADER_H
#define ICE_SHADER_H

#include <Asset.h>


namespace ICE {
class Shader : public Asset {
   public:
    std::string getTypeName() const override;

    AssetType getType() const override;
};
}  // namespace ICE

#endif  //ICE_SHADER_H
