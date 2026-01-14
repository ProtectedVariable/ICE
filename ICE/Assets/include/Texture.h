//
// Created by Thomas Ibanez on 22.12.20.
//

#ifndef ICE_TEXTURE_H
#define ICE_TEXTURE_H
#include <Asset.h>
#include <Logger.h>
#include <stb/stb_image.h>

#include <cstdint>
#include <string>

namespace ICE {
enum class TextureFormat { None = 0, SRGB8, SRGBA8, RGB8, RGBA8, Float16, MONO8 };

enum class TextureWrap { None = 0, Clamp = 1, Repeat = 2 };

enum class TextureType { Tex2D = 0, CubeMap = 1 };

class Texture : public Asset {
   public:
    virtual void bind(uint32_t slot = 0) const = 0;
    virtual const void* data() const = 0;
    virtual void setData(void* data, uint32_t size) = 0;

    virtual TextureFormat getFormat() const = 0;

    virtual uint32_t getWidth() const = 0;
    virtual uint32_t getHeight() const = 0;

    virtual void* getTexture() const = 0;

    virtual TextureType getTextureType() const = 0;

   protected:
    static void* getDataFromFile(const std::string file, int* width, int* height, int* channels, int force = STBI_default) {
        stbi_uc* data = stbi_load(file.c_str(), width, height, channels, force);
        if (data == nullptr) {
            Logger::Log(Logger::ERROR, "Graphics", "Texture %s could not load: %s", file.c_str(), stbi_failure_reason());
        }
        return data;
    }
};

class Texture2D : public Texture {
   public:
    virtual TextureWrap getWrap() const = 0;
    virtual TextureType getTextureType() const = 0;

    virtual AssetType getType() const override { return AssetType::ETex2D; }
    virtual std::string getTypeName() const override { return "Texture2D"; }
};

class TextureCube : public Texture {
   public:
    virtual TextureWrap getWrap() const = 0;
    virtual TextureType getTextureType() const = 0;

    virtual AssetType getType() const override { return AssetType::ETexCube; }
    virtual std::string getTypeName() const override { return "TextureCube"; }
};
}  // namespace ICE

#endif  //ICE_TEXTURE_H
