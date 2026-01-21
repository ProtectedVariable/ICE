//
// Created by Thomas Ibanez on 22.12.20.
//

#pragma once
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
    virtual ~Texture() {
        if (data_ != nullptr) {
            stbi_image_free(data_);
            data_ = nullptr;
        }
    }
    const void* data() const { return data_; }

    TextureFormat getFormat() const { return m_format; }
    TextureWrap getWrap() const { return m_wrap; }

    uint32_t getWidth() const { return m_width; }
    uint32_t getHeight() const { return m_height; }

   protected:
    static void* getDataFromFile(const std::string file, int* width, int* height, int* channels, int force = STBI_default) {
        stbi_uc* data = stbi_load(file.c_str(), width, height, channels, force);
        if (data == nullptr) {
            Logger::Log(Logger::ERROR, "Graphics", "Texture %s could not load: %s", file.c_str(), stbi_failure_reason());
        }
        return data;
    }

    void* data_ = nullptr;
    TextureWrap m_wrap = TextureWrap::Repeat;
    TextureFormat m_format = TextureFormat::None;
    int m_width = 0;
    int m_height = 0;
};

class Texture2D : public Texture {
   public:
    Texture2D(const std::string& path);
    Texture2D(void* data, int width, int height, TextureFormat fmt);

    virtual AssetType getType() const override { return AssetType::ETex2D; }
    virtual std::string getTypeName() const override { return "Texture2D"; }
};

class TextureCube : public Texture {
   public:
    TextureCube(const std::string& path);
    TextureCube(void* data, int width, int height, TextureFormat fmt);

    virtual AssetType getType() const override { return AssetType::ETexCube; }
    virtual std::string getTypeName() const override { return "TextureCube"; }
};
}  // namespace ICE