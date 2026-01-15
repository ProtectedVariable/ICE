//
// Created by Thomas Ibanez on 22.12.20.
//

#include <GL/gl3w.h>
#include <ICEException.h>

#include "OpenGLTexture.h"

namespace ICE {

OpenGLTexture2D::OpenGLTexture2D(const Texture2D &tex) {
    auto width = tex.getWidth();
    auto height = tex.getHeight();

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    auto fmt = tex.getFormat();
    auto storageFormat = textureFormatToGLInternalFormat(fmt);
    auto dataFormat = (textureFormatToChannels(fmt) == 4) ? GL_RGBA : (textureFormatToChannels(fmt) == 3) ? GL_RGB : GL_RED;
    glPixelStorei(GL_UNPACK_ALIGNMENT, textureFormatToAlignment(fmt));

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    auto wrap = tex.getWrap();
    auto glWrap = (wrap == TextureWrap::Clamp) ? GL_CLAMP_TO_EDGE : (wrap == TextureWrap::Repeat) ? GL_REPEAT : GL_REPEAT;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrap);

    glTexImage2D(GL_TEXTURE_2D, 0, storageFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, tex.data());
}

int OpenGLTexture2D::id() const {
    return m_id;
}

void OpenGLTexture2D::bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_id);
}
}  // namespace ICE