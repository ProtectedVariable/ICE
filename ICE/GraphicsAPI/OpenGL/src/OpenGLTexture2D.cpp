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

OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat fmt) {
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    auto storageFormat = textureFormatToGLInternalFormat(fmt);
    auto channels = textureFormatToChannels(fmt);
    auto dataFormat = (channels == 4) ? GL_RGBA : (channels == 3) ? GL_RGB : GL_RED;
    glPixelStorei(GL_UNPACK_ALIGNMENT, textureFormatToAlignment(fmt));

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Clamp: a graph target is sampled full-screen, where repeat would wrap edge taps.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Null data: allocate storage only. The pixel type still has to agree with the internal
    // format, so a half-float target uploads as GL_FLOAT rather than GL_UNSIGNED_BYTE.
    const GLenum pixel_type = (fmt == TextureFormat::Float16) ? GL_FLOAT : GL_UNSIGNED_BYTE;
    glTexImage2D(GL_TEXTURE_2D, 0, storageFormat, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, dataFormat, pixel_type,
                 nullptr);
}

OpenGLTexture2D::~OpenGLTexture2D() {
    glDeleteTextures(1, &m_id);
}

int OpenGLTexture2D::id() const {
    return m_id;
}

void OpenGLTexture2D::bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_id);
}
}  // namespace ICE