#pragma once

#include <glm/glm.hpp>

class OrbitCamera {
public:
    OrbitCamera();

    void SetTarget(glm::vec3 target, float distance, float yawDegrees, float pitchDegrees);
    void Orbit(float deltaX, float deltaY);
    void Pan(float deltaX, float deltaY);
    void Zoom(float delta);

    glm::mat4 ViewMatrix() const;
    glm::mat4 ProjectionMatrix(float aspectRatio) const;

    glm::vec3 Position() const;
    glm::vec3 Target() const;
    float Distance() const;

private:
    glm::vec3 target_;
    float distance_;
    float yawDegrees_;
    float pitchDegrees_;
};
