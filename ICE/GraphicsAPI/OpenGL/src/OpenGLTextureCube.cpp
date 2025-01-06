//
// Created by Thomas Ibanez on 29.12.20.
//
#include <ICEMath.h>

#include <Eigen/Dense>

#include "OpenGLTexture.h"

namespace ICE {

OpenGLTextureCube::OpenGLTextureCube(const std::string &file) {
    int channels, w, h;
    void *data = Texture::getDataFromFile(file, &w, &h, &channels, STBI_rgb);
    width = w;
    height = h;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    auto faces = equirectangularToCubemap((uint8_t *) data, width, height);
    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width / 4, width / 4, 0, GL_RGB, GL_UNSIGNED_BYTE, faces[i]);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);
}

void OpenGLTextureCube::bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

TextureFormat OpenGLTextureCube::getFormat() const {
    return TextureFormat::RGB;
}

uint32_t OpenGLTextureCube::getWidth() const {
    return width;
}

uint32_t OpenGLTextureCube::getHeight() const {
    return height;
}

TextureWrap OpenGLTextureCube::getWrap() const {
    return TextureWrap::Clamp;
}

void OpenGLTextureCube::setData(void *data, uint32_t size) {
    //TODO
}

void *OpenGLTextureCube::getTexture() const {
    return static_cast<char *>(0) + id;
}

TextureType OpenGLTextureCube::getTextureType() const {
    return TextureType::CubeMap;
}
}  // namespace ICE