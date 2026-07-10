#include "Texture.h"

namespace ICE {

Texture2D::Texture2D(const std::string& path) {
    int channels = 0;
    data_ = getDataFromFile(path, &m_width, &m_height, &channels);
    m_owns_data = true;  // stb-allocated; freed in ~Texture
    if (channels == 3) {
        m_format = TextureFormat::RGB8;
    } else if (channels == 4) {
        m_format = TextureFormat::RGBA8;
    } else if (channels == 1) {
        m_format = TextureFormat::MONO8;
    } else {
        m_format = TextureFormat::None;
    }
}
Texture2D::Texture2D(void* data, int width, int height, TextureFormat fmt, bool take_ownership) {
    data_ = data;
    m_owns_data = take_ownership;
    m_width = width;
    m_height = height;
    m_format = fmt;
}

TextureCube::TextureCube(const std::string& path) {
    // Load the equirectangular source image as RGB; OpenGLTextureCube converts it into the
    // six cube faces. This used to be an empty stub, so cubemaps loaded from a path had null
    // data and zero size.
    int channels = 0;
    data_ = getDataFromFile(path, &m_width, &m_height, &channels, STBI_rgb);
    m_owns_data = true;  // stb-allocated; freed in ~Texture
    m_format = TextureFormat::RGB8;
}
TextureCube::TextureCube(void* data, int width, int height, TextureFormat fmt) {
    data_ = data;
    m_width = width;
    m_height = height;
    m_format = fmt;
}
}  // namespace ICE
