# pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

static constexpr float PIXEL_TO_RAD = 0.01f;

class Camera {
public:
    // attributes
    glm::vec3 position;
    glm::vec3 front, up, right; // needed for view matrix

    float radius = 5.0f;    // distance
    float theta = 0.0f;     // horizontal
    float pi = 0.0f;        // vertical

    float sensitivity = 0.18f;
    float fov = 36.0f;

    glm::vec3 target = glm::vec3(0.0f); // world-center

    // TODO: move function definitions into a Camera.cpp file

    // constructor
    Camera(float radius, float theta, float phi, glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f) 
        : radius(radius), theta(theta), phi(phi), m_worldUp(m_worldUp) {
        update();
    }

    // this should be called every frame
    void update() {
        position = calculatePosition();
        updateVectors();
    }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, target, up);
    }
    glm::mat4 getProjectionMatrix(float width, float height) const {
        return glm::perspective(glm::radians(fov), width / height, 0.1f, 100.0f);
    }

    void rotate(float xOffset, float yOffset) {
        theta += xOffset * sensitivity * PIXEL_TO_RAD;
    }

private:
    glm::vec3 m_worldUp;

    // TODO: move these function definitions into a Camera.cpp file

    glm::vec3 calculatePosition() const {
        // just return spherical to cartesian conversion
        return glm::vec3(
            radius * cos(phi) * cos(theta),
            radius * sin(phi),
            radius * cos(phi) * sin(theta)
        );
    }

    // https://www.songho.ca/opengl/gl_camera.html
    // these vectors are needed for constructing the view matrix
    void updateVectors() {
        front = glm::normalize(-position);
        right = glm::normalize(glm::cross(front, m_worldUp));
        up = glm::normalize(glm::cross(right, front));
    }

};