# pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <logger.h>

static constexpr float PIXEL_TO_RAD = 0.01f;

class Camera {
public:
    // attributes
    glm::vec3 position;
    glm::vec3 front, up, right; // needed for view matrix

    float radius = 5.0f;    // distance
    float theta = 0.0f;     // horizontal
    float phi = 0.0f;        // vertical

    float sensitivity = 0.18f;
    float fov = 36.0f;

    glm::vec3 target = glm::vec3(0.0f); // world-center

    // constructor
    Camera(float radius, float theta, float pi, glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f));

    // this should be called every frame
    void update();

    // this should be called when viewport dimensions change
    // i.e. when window size changes
    void setViewport(int width, int height);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    void rotate(float xOffset, float yOffset);
    void zoom(float offset);
    void reset();

    void setTarget(const glm::vec3& newTarget);

private:
    glm::vec3 m_worldUp;
    int m_viewportWidth = 800;
    int m_viewportHeight = 600;

    glm::vec3 calculatePosition() const;
    void updateVectors();

};