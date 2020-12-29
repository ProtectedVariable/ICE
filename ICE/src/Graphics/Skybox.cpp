//
// Created by Thomas Ibanez on 29.12.20.
//

#include "Skybox.h"

namespace ICE {

    static VertexArray* vao;
    static Shader* shader;

    Skybox::Skybox(Texture *texture) : texture(texture) {}

    VertexArray *Skybox::getVAO() {
        return vao;
    }

    Texture *Skybox::getTexture() const {
        return texture;
    }

    Shader *Skybox::getShader() {
        return shader;
    }

    void Skybox::Initialize() {
        shader = Shader::Create("Assets/Shaders/skybox.vs","Assets/Shaders/skybox.fs");
        vao = VertexArray::Create();
        static VertexBuffer* vbo = VertexBuffer::Create();
        vbo->putData(skyboxVertices, 3 * 36 * sizeof(float));
        vao->pushVertexBuffer(vbo, 3);
    }
}
