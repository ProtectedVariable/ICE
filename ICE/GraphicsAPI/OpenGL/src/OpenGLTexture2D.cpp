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

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    auto fmt = tex.getFormat();
    auto storageFormat = textureFormatToGLInternalFormat(fmt);
    auto dataFormat = (textureFormatToChannels(fmt) == 4) ? GL_RGBA : (textureFormatToChannels(fmt) == 3) ? GL_RGB : GL_RED;
    glPixelStorei(GL_UNPACK_ALIGNMENT, textureFormatToAlignment(fmt));

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    auto wrap = TextureWrap::Repeat;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, storageFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, tex.data());
}

void OpenGLTexture2D::bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
}
}  // namespace ICE