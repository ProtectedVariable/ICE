//
// Created by Thomas Ibanez on 22.12.20.
//

#ifndef ICE_OPENGLTEXTURE_H
#define ICE_OPENGLTEXTURE_H

#include <GL/gl3w.h>
#include <Texture.h>

#include <string>

namespace ICE {
class OpenGLTexture2D : public Texture2D {
   public:
    OpenGLTexture2D(const std::string &file);
    OpenGLTexture2D(const void *data, size_t w, size_t h, TextureFormat fmt);

    void bind(uint32_t slot) const override;

    TextureFormat getFormat() const override;

    uint32_t getWidth() const override;
    uint32_t getHeight() const override;

    TextureWrap getWrap() const override;

    void setData(void *data, uint32_t size) override;

    void *getTexture() const override;

    TextureType getTextureType() const override;

   private:
    std::string file;
    uint32_t id;
    uint32_t width, height;
    TextureFormat format;
    TextureWrap wrap;
    GLenum storageFormat;
    GLenum dataFormat;
};

class OpenGLTextureCube : public TextureCube {
   public:
    OpenGLTextureCube(const std::string &file);

    void bind(uint32_t slot) const override;

    TextureFormat getFormat() const override;

    uint32_t getWidth() const override;
    uint32_t getHeight() const override;

    TextureWrap getWrap() const override;

    void setData(void *data, uint32_t size) override;

    void *getTexture() const override;

    TextureType getTextureType() const override;

   private:
    std::string file;
    uint32_t id;
    uint32_t width, height;
    TextureFormat format;
    TextureWrap wrap;
    GLenum storageFormat;
    GLenum dataFormat;
};
}  // namespace ICE

#endif  //ICE_OPENGLTEXTURE_H
