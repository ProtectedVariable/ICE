//
// Created by Thomas Ibanez on 22.12.20.
//

#ifndef ICE_OPENGLTEXTURE_H
#define ICE_OPENGLTEXTURE_H

#include <GL/gl3w.h>
#include <GPUTexture.h>
#include <Texture.h>

#include <string>

namespace ICE {

constexpr GLenum textureFormatToGLInternalFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::SRGB8:
            return GL_SRGB8;
        case TextureFormat::SRGBA8:
            return GL_SRGB8_ALPHA8;
        case TextureFormat::RGB8:
            return GL_RGB8;
        case TextureFormat::RGBA8:
            return GL_RGBA8;
        case TextureFormat::Float16:
            return GL_RGBA16F;
        case TextureFormat::MONO8:
            return GL_R8;
        default:
            return GL_RGBA8;
    }
}

constexpr int textureFormatToAlignment(TextureFormat format) {
    switch (format) {
        case TextureFormat::SRGB8:
        case TextureFormat::RGB8:
            return 1;
        case TextureFormat::SRGBA8:
        case TextureFormat::RGBA8:
        case TextureFormat::Float16:
            return 4;
        case TextureFormat::MONO8:
            return 1;
        default:
            return 4;
    }
}

constexpr int textureFormatToChannels(TextureFormat format) {
    switch (format) {
        case TextureFormat::SRGB8:
        case TextureFormat::RGB8:
            return 3;
        case TextureFormat::SRGBA8:
        case TextureFormat::RGBA8:
        case TextureFormat::Float16:
            return 4;
        case TextureFormat::MONO8:
            return 1;
        default:
            return 4;
    }
}

class OpenGLTexture2D : public GPUTexture {
   public:
    OpenGLTexture2D(const Texture2D &tex);

    void bind(uint32_t slot) const override;

   private:
    uint32_t id;
};

class OpenGLTextureCube : public GPUTexture {
   public:
    OpenGLTextureCube(const TextureCube &tex);

    void bind(uint32_t slot) const override;

   private:
    uint32_t id;
};
}  // namespace ICE

#endif  //ICE_OPENGLTEXTURE_H
