//
// Created by Thomas Ibanez on 29.12.20.
//
#include <ICEMath.h>

#include <Eigen/Dense>

#include "OpenGLTexture.h"

namespace ICE {

OpenGLTextureCube::OpenGLTextureCube(const TextureCube &texture_asset) {
   
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

    auto width = texture_asset.getWidth();
    auto faces = equirectangularToCubemap((uint8_t *) texture_asset.data(), width, texture_asset.getHeight());
    // RGB (3-byte) rows: without alignment 1 the default of 4 shears any face whose width
    // is not a multiple of 4.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, width / 4, width / 4, 0, GL_RGB, GL_UNSIGNED_BYTE, faces[i]);
    }
    // equirectangularToCubemap allocates the six faces with new[]; free them after upload.
    for (int i = 0; i < 6; i++) {
        delete[] faces[i];
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}


OpenGLTextureCube::~OpenGLTextureCube() {
    glDeleteTextures(1, &m_id);
}

int OpenGLTextureCube::id() const {
    return m_id;
}

void OpenGLTextureCube::bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
}
}  // namespace ICE