#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;

// defaults
const float d_radius = 5.0f; // distance from target
const float d_theta = 0.0f;
const float d_phi = 0.0f;

const float d_sensitivity = 0.18f;
const float d_dof = 36.0f;

const glm::vec3 d_target = glm::vec3(0.0f, 0.0f, 0.0f);

class Camera {
public:
  // attributes
  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;
  glm::vec3 worldUp;

  glm::vec3 target;

  float radius, theta, phi;
  float sensitivity, dof; // dof in degrees

  static constexpr float PIXEL_TO_RAD = 0.01f;

  // constructor
  Camera(
    float radius = d_radius,
    float theta = d_theta,
    float phi = d_phi,
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)
  ) : sensitivity(d_sensitivity), dof(d_dof), target(d_target) {
    this->position = calculatePosition(radius, theta, phi);
    this->worldUp = up;
    this->radius = radius;
    this->theta = theta;
    this->phi = phi;
    updateVectors();
  }

  void update() {
    position = calculatePosition(radius, theta, phi);
    updateVectors();
  }

  // view matrix getter
  glm::mat4 getViewMatrix() {
    return glm::lookAt(position, target, up);
  }
  // projection matrix getter
  glm::mat4 getProjectionMatrix(float SCR_WIDTH, float SCR_HEIGHT) {
    return glm::perspective(glm::radians(dof), SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
  }

  void rotate(float xOff, float yOff) {
    theta += xOff * sensitivity * PIXEL_TO_RAD;
    phi += yOff * sensitivity * PIXEL_TO_RAD;

    // constraints
    if (phi > glm::radians(89.0f))
      phi = glm::radians(89.0f);
    if (phi < glm::radians(-89.0f))
      phi = glm::radians(-89.0f);

    updateVectors();
  }

  void zoom(float offset) {
    radius -= offset * 0.32f;
    if (radius < 0.2f)
      radius = 0.2f;
    if (radius > 700.0f)
      radius = 700.0f;
  }

  // reset position
  void reset() {
    // just reset object values
    // position should be updated next frame (on next update() call)
    radius = d_radius;
    theta = d_theta;
    phi = d_phi;

    std::cout << "Camera Reset\n";
  }

private:
  glm::vec3 calculatePosition(float radius, float theta, float phi) {
    return glm::vec3(
      radius * cos(phi) * cos(theta),
      radius * sin(phi),
      radius * cos(phi) * sin(theta)
    );
  }

  void updateVectors() {
    // front points camera to target
    front = glm::normalize(-position);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
  }
};

#endif // !CAMERA_HPP
