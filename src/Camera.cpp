#include "Camera.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace {
constexpr float kMinPitch = -89.0f;
constexpr float kMaxPitch = 89.0f;
}

OrbitCamera::OrbitCamera()
    : target_(0.0f, 0.0f, 0.0f), distance_(6.0f), yawDegrees_(45.0f), pitchDegrees_(-25.0f) {
}

void OrbitCamera::SetTarget(glm::vec3 target, float distance, float yawDegrees, float pitchDegrees) {
    target_ = target;
    distance_ = distance;
    yawDegrees_ = yawDegrees;
    pitchDegrees_ = std::clamp(pitchDegrees, kMinPitch, kMaxPitch);
}

void OrbitCamera::Orbit(float deltaX, float deltaY) {
    yawDegrees_ -= deltaX * 0.35f;
    pitchDegrees_ -= deltaY * 0.35f;
    pitchDegrees_ = std::clamp(pitchDegrees_, kMinPitch, kMaxPitch);
}

void OrbitCamera::Pan(float deltaX, float deltaY) {
    const glm::vec3 cameraPos = Position();
    const glm::vec3 forward = glm::normalize(target_ - cameraPos);
    const glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    const glm::vec3 up = glm::normalize(glm::cross(right, forward));
    target_ += right * (-deltaX * 0.01f * distance_);
    target_ += up * (deltaY * 0.01f * distance_);
}

void OrbitCamera::Move(float forward, float right, float up, float speed) {
    const glm::vec3 cameraPos = Position();
    const glm::vec3 viewDir = glm::normalize(target_ - cameraPos);
    const glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 cameraRight = glm::normalize(glm::cross(viewDir, worldUp));
    const glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, viewDir));

    target_ += viewDir * forward * speed;
    target_ += cameraRight * right * speed;
    target_ += cameraUp * up * speed;
}

void OrbitCamera::MoveForward(float amount) {
    const glm::vec3 cameraPos = Position();
    const glm::vec3 viewDir = glm::normalize(target_ - cameraPos);
    target_ += viewDir * amount;
}

void OrbitCamera::MoveSideways(float amount) {
    const glm::vec3 cameraPos = Position();
    const glm::vec3 viewDir = glm::normalize(target_ - cameraPos);
    const glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 cameraRight = glm::normalize(glm::cross(viewDir, worldUp));
    target_ += cameraRight * amount;
}

void OrbitCamera::MoveUp(float amount) {
    target_ += glm::vec3(0.0f, amount, 0.0f);
}

void OrbitCamera::Zoom(float delta) {
    distance_ *= std::pow(1.12f, -delta * 0.01f);
    distance_ = std::clamp(distance_, 1.5f, 30.0f);
}

glm::mat4 OrbitCamera::ViewMatrix() const {
    return glm::lookAt(Position(), target_, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 OrbitCamera::ProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(60.0f), aspectRatio, 0.01f, 100.0f);
}

glm::vec3 OrbitCamera::Position() const {
    const float yaw = glm::radians(yawDegrees_);
    const float pitch = glm::radians(pitchDegrees_);
    const glm::vec3 direction(
        std::cos(yaw) * std::cos(pitch),
        std::sin(pitch),
        std::sin(yaw) * std::cos(pitch));
    return target_ + direction * distance_;
}

glm::vec3 OrbitCamera::Target() const {
    return target_;
}

float OrbitCamera::Distance() const {
    return distance_;
}
