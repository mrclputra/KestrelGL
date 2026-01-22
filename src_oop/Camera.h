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

    float yaw = -90.0f;
    float pitch = 0.0f;

    //float radius = 2.0f;    // distance
    //float theta = 0.0f;     // horizontal
    //float phi = 0.0f;        // vertical

    float sensitivity = 0.09f;
    float speed = 5.0f;
    float fov = 36.0f;

    // constructor
    Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f));

    // this should be called every frame
    void updateVectors();

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    void rotate(float xOffset, float yOffset);
    void moveForward(float dt);
    void moveBackward(float dt);
    void moveRight(float dt);
    void moveLeft(float dt);
    void moveUp(float dt);
    void moveDown(float dt);

    // this should be called when viewport dimensions change
    // i.e. when window size changes
    void setViewport(int width, int height);

private:
    glm::vec3 m_worldUp;
    int m_viewportWidth = 800;
    int m_viewportHeight = 600;
};