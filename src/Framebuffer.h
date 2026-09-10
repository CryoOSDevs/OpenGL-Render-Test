#pragma once

#include <glm/glm.hpp>

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    void Resize(int width, int height);
    void Bind() const;
    void Unbind() const;
    unsigned int TextureID() const;
    glm::ivec2 Size() const;
    void Destroy();

private:
    unsigned int framebuffer_ = 0;
    unsigned int colorTexture_ = 0;
    unsigned int depthRenderbuffer_ = 0;
    int width_ = 0;
    int height_ = 0;
};
