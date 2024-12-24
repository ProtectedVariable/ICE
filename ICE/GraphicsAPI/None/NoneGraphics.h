//
// Created by Thomas Ibanez on 24.02.21.
//

#ifndef ICE_NONEGRAPHICS_H
#define ICE_NONEGRAPHICS_H

#include <Buffers.h>

#include <Eigen/Dense>

namespace ICE {

class NoneVertexBuffer : public VertexBuffer {
   public:
    void bind() const override {}
    void unbind() const override {}
    void putData(const void *data, uint32_t size) override {}
    uint32_t getSize() const override { return 0; }
};

class NoneIndexBuffer : public IndexBuffer {
   public:
    void bind() const override {}
    void unbind() const override {}
    uint32_t getSize() const override { return 0; }
    void putData(const void *data, uint32_t size) override {}
};

class NoneContext : public Context {
   public:
    void swapBuffers() override {}
    void wireframeMode() override {}
    void fillMode() override {}
    void initialize() override {}
};

class NoneFramebuffer : public Framebuffer {
   public:
    NoneFramebuffer() : Framebuffer({0,0,0}) {}
    void bind() override {}
    void unbind() override {}
    void resize(int width, int height) override {}
    int getTexture() override { return 0; }
    void bindAttachment(int slot) const override{};
    Eigen::Vector4i readPixel(int x, int y) override { return Eigen::Vector4i(); }
};

class NoneRendererAPI : public RendererAPI {
   public:
    void initialize() const override {}
    void setViewport(int x, int y, int width, int height) const override {}
    void setClearColor(float r, float g, float b, float a) const override {}
    void clear() const override {}
    void renderVertexArray(const std::shared_ptr<VertexArray> &va) const override {}
    void flush() const override {}
    void finish() const override {}
    void bindDefaultFramebuffer() const override {}
    void setDepthTest(bool enable) const override {}
    void setDepthMask(bool enable) const override {}
    void setBackfaceCulling(bool enable) const override {}
    void checkAndLogErrors() const override {}
};

class NoneShader : public Shader {
   public:
    void bind() const override {}
    void unbind() const override {}
    void loadInt(const std::string &name, int v) override {}
    void loadInts(const std::string &name, int *array, uint32_t size) override {}
    void loadFloat(const std::string &name, float v) override {}
    void loadFloat2(const std::string &name, Eigen::Vector2f vec) override {}
    void loadFloat3(const std::string &name, Eigen::Vector3f vec) override {}
    void loadFloat4(const std::string &name, Eigen::Vector4f vec) override {}
    void loadMat4(const std::string &name, Eigen::Matrix4f mat) override {}
    virtual AssetType getType() const override { return AssetType::EShader; }
    virtual std::string getTypeName() const override { return "Shader"; }
};

class NoneTexture2D : public Texture2D {
   public:
    void bind(uint32_t slot) const override {}
    TextureFormat getFormat() const override { return {}; }
    uint32_t getWidth() const override { return 0; }
    uint32_t getHeight() const override { return 0; }
    TextureWrap getWrap() const override { return {}; }
    void setData(void *data, uint32_t size) override {}
    void *getTexture() const override { return nullptr; }
    TextureType getTextureType() const override { return TextureType::Tex2D; }
};

class NoneTextureCube : public TextureCube {
   public:
    void bind(uint32_t slot) const override {}
    TextureFormat getFormat() const override { return {}; }
    uint32_t getWidth() const override { return 0; }
    uint32_t getHeight() const override { return 0; }
    TextureWrap getWrap() const override { return {}; }
    void setData(void *data, uint32_t size) override {}
    void *getTexture() const override { return nullptr; }
    TextureType getTextureType() const override { return TextureType::CubeMap; }
};

class NoneVertexArray : public VertexArray {
   public:
    void bind() const override {}
    void unbind() const override {}
    void pushVertexBuffer(const std::shared_ptr<VertexBuffer> &buffer, int size) override {}
    void pushVertexBuffer(const std::shared_ptr<VertexBuffer> &buffer, int position, int size) override {}
    void setIndexBuffer(const std::shared_ptr<IndexBuffer> &buffer) override {}
    int getIndexCount() const override { return 0; }
    uint32_t getID() const override { return 0; }
    std::shared_ptr<IndexBuffer> getIndexBuffer() const override { return nullptr; }
};
}  // namespace ICE

#endif  //ICE_NONEGRAPHICS_H
