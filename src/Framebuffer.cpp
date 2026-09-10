#include "Framebuffer.h"

#include <glad/glad.h>
#include <stdexcept>

Framebuffer::Framebuffer() = default;

Framebuffer::~Framebuffer() {
    if (colorTexture_ != 0) {
        glDeleteTextures(1, &colorTexture_);
    }
    if (depthRenderbuffer_ != 0) {
        glDeleteRenderbuffers(1, &depthRenderbuffer_);
    }
    if (framebuffer_ != 0) {
        glDeleteFramebuffers(1, &framebuffer_);
    }
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : framebuffer_(other.framebuffer_), colorTexture_(other.colorTexture_), depthRenderbuffer_(other.depthRenderbuffer_), width_(other.width_), height_(other.height_) {
    other.framebuffer_ = 0;
    other.colorTexture_ = 0;
    other.depthRenderbuffer_ = 0;
    other.width_ = 0;
    other.height_ = 0;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
    if (this != &other) {
        if (colorTexture_ != 0) glDeleteTextures(1, &colorTexture_);
        if (depthRenderbuffer_ != 0) glDeleteRenderbuffers(1, &depthRenderbuffer_);
        if (framebuffer_ != 0) glDeleteFramebuffers(1, &framebuffer_);
        framebuffer_ = other.framebuffer_;
        colorTexture_ = other.colorTexture_;
        depthRenderbuffer_ = other.depthRenderbuffer_;
        width_ = other.width_;
        height_ = other.height_;
        other.framebuffer_ = 0;
        other.colorTexture_ = 0;
        other.depthRenderbuffer_ = 0;
        other.width_ = 0;
        other.height_ = 0;
    }
    return *this;
}

void Framebuffer::Resize(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }
    if (width_ == width && height_ == height && framebuffer_ != 0) {
        return;
    }

    if (framebuffer_ == 0) {
        glGenFramebuffers(1, &framebuffer_);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

    if (colorTexture_ == 0) {
        glGenTextures(1, &colorTexture_);
    }
    glBindTexture(GL_TEXTURE_2D, colorTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture_, 0);

    if (depthRenderbuffer_ == 0) {
        glGenRenderbuffers(1, &depthRenderbuffer_);
    }
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer_);

    const GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Framebuffer incomplete");
    }

    width_ = width;
    height_ = height;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Bind() const {
    if (framebuffer_ == 0) {
        throw std::runtime_error("Attempted to bind an uninitialized framebuffer");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
}

void Framebuffer::Unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Framebuffer::TextureID() const {
    return colorTexture_;
}

glm::ivec2 Framebuffer::Size() const {
    return glm::ivec2(width_, height_);
}
