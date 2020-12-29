//
// Created by Thomas Ibanez on 22.12.20.
//

#ifndef ICE_TEXTURE_H
#define ICE_TEXTURE_H

#include <cstdint>
#include <string>
#include <stb/stb_image.h>


namespace ICE {
    enum class TextureFormat
    {
        None = 0,
        RGB = 1,
        RGBA = 2,
        Float16 = 3
    };

    enum class TextureWrap
    {
        None = 0,
        Clamp = 1,
        Repeat = 2
    };

    class Texture {
    public:
        virtual void bind(uint32_t slot = 0) const = 0;
        virtual void setData(void* data, uint32_t size) = 0;

        virtual TextureFormat getFormat() const = 0;

        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;

        virtual void* getTexture() const = 0;

        static void* getDataFromFile(const std::string file, int* width, int* height, int* channels, int force = STBI_default) {
            stbi_set_flip_vertically_on_load(1);
            stbi_uc* data = stbi_load(file.c_str(), width, height, channels, force);
            return data;
        }
    };

    class Texture2D : public Texture {
    public:
        virtual TextureWrap getWrap() const = 0;
        static Texture2D* Create(const std::string& file);
    };

    class TextureCube : public Texture {
    public:
        virtual TextureWrap getWrap() const = 0;
        static TextureCube* Create(const std::string& file);
    };
}


#endif //ICE_TEXTURE_H
