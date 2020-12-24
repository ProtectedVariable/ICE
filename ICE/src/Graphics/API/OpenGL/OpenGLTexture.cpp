//
// Created by Thomas Ibanez on 22.12.20.
//

#include "OpenGLTexture.h"
#include <GL/gl3w.h>
#include <stb/stb_image.h>
#include <Util/ICEException.h>

namespace ICE {

    OpenGLTexture2D::OpenGLTexture2D(const std::string &file) : file(file) {
        stbi_set_flip_vertically_on_load(1);
        int w,h,channels;
        stbi_uc* data = stbi_load(file.c_str(), &w, &h, &channels, 0);
        width = w;
        height = h;
        storageFormat = GL_RGBA;
        dataFormat = GL_RGBA;
        if(channels == 4) {
            format = TextureFormat::RGBA;
        } else if(channels == 3) {
            storageFormat = dataFormat = GL_RGB;
            format = TextureFormat::RGB;
        }

        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        wrap = TextureWrap::Repeat;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, storageFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    void OpenGLTexture2D::setData(void *data, uint32_t size) {
            uint32_t bpp = (format == TextureFormat::RGBA) ? 4 : 3;
            if(size != bpp * width * height) {
                throw ICEException();
            }
            glTextureSubImage2D(id, 0,0,0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);
    }

    void OpenGLTexture2D::bind(uint32_t slot) const {
        glBindTextureUnit(slot, id);
    }

    TextureWrap OpenGLTexture2D::getWrap() const {
        return wrap;
    }

    TextureFormat OpenGLTexture2D::getFormat() const {
        return format;
    }

    uint32_t OpenGLTexture2D::getWidth() const {
        return width;
    }

    uint32_t OpenGLTexture2D::getHeight() const {
        return height;
    }
}