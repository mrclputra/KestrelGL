#pragma once

// TODO: review, should I move implementation to source?

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// TODO: define default values outside class definition
const float PIXEL_TO_RAD = 0.01f;

class Camera {
public:
    // attributes
    glm::vec3 position;
    glm::vec3 front, up, right;
    glm::vec3 target = glm::vec3(0.0f);

    float radius = 5.0f;
    float theta = 0.0f;
    float phi = 0.0f;

    float sensitivity = 0.18f;
    float fov = 36.0f;

    Camera(float radius = 5.0f, float theta = 0.0f, float phi = 0.0f, glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f))
        : radius(radius), theta(glm::radians(theta)), phi(glm::radians(phi)), m_worldUp(worldUp) {
        update(); // first time update
    }

    void update() {
        position = calculatePosition();
        updateVectors();
    }

    // this should be called when viewport dimensions change
    void setViewport(int width, int height) {
        m_viewportWidth = width;
        m_viewportHeight = height;
    }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, target, up);
    }
    glm::mat4 getProjectionMatrix() const {
        return glm::perspective(glm::radians(fov), (float)m_viewportWidth / (float)m_viewportHeight, 0.1f, 100.0f);
    }

    void rotate(float xOffset, float yOffset) {
        theta += xOffset * sensitivity * PIXEL_TO_RAD;
        phi = glm::clamp(phi + yOffset * sensitivity * 0.01f,
            glm::radians(-89.0f), glm::radians(89.0f));
    }
    void zoom(float offset) {
        radius = glm::clamp(radius - offset * 0.32f, 0.2f, 700.0f);
    }
    void reset() {
        radius = 5.0f;
        theta = 0.0f;
        phi = 0.0f;
        std::cout << "Camera reset\n";
    }

private:
    glm::vec3 m_worldUp;
    int m_viewportWidth = 800;
    int m_viewportHeight = 600;

    glm::vec3 calculatePosition() const {
        return glm::vec3(
            radius * cos(phi) * cos(theta),
            radius * sin(phi),
            radius * cos(phi) * sin(theta)
        );
    }

    void updateVectors() {
        front = glm::normalize(-position);
        right = glm::normalize(glm::cross(front, m_worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};