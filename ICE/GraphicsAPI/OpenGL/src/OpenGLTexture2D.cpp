//
// Created by Thomas Ibanez on 22.12.20.
//

#include <GL/gl3w.h>
#include <ICEException.h>

#include "OpenGLTexture.h"

namespace ICE {

OpenGLTexture2D::OpenGLTexture2D(const std::string &file) : file(file) {
    int channels, w, h;
    void *data = Texture::getDataFromFile(file, &w, &h, &channels);
    loadData(data, w, h, channels == 4 ? TextureFormat::RGBA8 : TextureFormat::RGB8);
    stbi_image_free(data);
}

OpenGLTexture2D::OpenGLTexture2D(const void *data, size_t w, size_t h, TextureFormat fmt) {
    loadData(data, w, h, fmt);
}

void OpenGLTexture2D::loadData(const void *data, size_t w, size_t h, TextureFormat fmt) {
    width = w;
    height = h;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    storageFormat = textureFormatToGLInternalFormat(fmt);
    dataFormat = (textureFormatToChannels(fmt) == 4) ? GL_RGBA : (textureFormatToChannels(fmt) == 3) ? GL_RGB : GL_RED;
    glPixelStorei(GL_UNPACK_ALIGNMENT, textureFormatToAlignment(fmt));

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    wrap = TextureWrap::Repeat;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, storageFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
}

void OpenGLTexture2D::setData(void *data, uint32_t size) {
    glTextureSubImage2D(id, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);
}

void OpenGLTexture2D::bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
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

void *OpenGLTexture2D::getTexture() const {
    return static_cast<char *>(0) + id;
}

TextureType OpenGLTexture2D::getTextureType() const {
    return TextureType::Tex2D;
}
}  // namespace ICE