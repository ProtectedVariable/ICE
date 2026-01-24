#include "Texture.h"

namespace ICE {

Texture2D::Texture2D(const std::string& path) {
    int channels = 0;
    data_ = getDataFromFile(path, &m_width, &m_height, &channels);
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
Texture2D::Texture2D(void* data, int width, int height, TextureFormat fmt) {
    data_ = data;
    m_width = width;
    m_height = height;
    m_format = fmt;
}

TextureCube::TextureCube(const std::string& path) {
}
TextureCube::TextureCube(void* data, int width, int height, TextureFormat fmt) {
    data_ = data;
    m_width = width;
    m_height = height;
    m_format = fmt;
}
}  // namespace ICE
