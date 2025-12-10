#include "Camera.h"

Camera::Camera(float radius, float theta, float pi, glm::vec3 worldUp ) {
	this->radius = radius;
	this->theta = theta;
	this->pi = pi;
	this->m_worldUp = worldUp;

	logger.info("camera created");
	update();
}

void Camera::update() {
	position = calculatePosition();
	updateVectors();
}
void Camera::setViewport(int width, int height) {
	m_viewportWidth = width;
	m_viewportHeight = height;
}

glm::mat4 Camera::getProjectionMatrix() const {
	return glm::perspective(glm::radians(fov), (float)m_viewportWidth / (float)m_viewportHeight, 0.1f, 100.0f);
}
glm::mat4 Camera::getViewMatrix() const {
	return glm::lookAt(position, target, up);
}

void Camera::rotate(float xOffset, float yOffset) {
	theta += xOffset * sensitivity * PIXEL_TO_RAD;
}
void Camera::zoom(float offset) {
	radius = glm::clamp(radius - offset * 0.32f, 0.2f, 700.0f);
}
void Camera::reset() {
	radius = 5.0f;
	theta = 0.0f;
	pi = 0.0f;
	logger.info("camera reset");
}

glm::vec3 Camera::calculatePosition() const {
	// spherical to cartesian
	return glm::vec3(
		radius * cos(pi) * cos(theta),
		radius * sin(pi),
		radius * cos(pi) * sin(theta)
	);
}

// https://www.songho.ca/opengl/gl_camera.html
void Camera::updateVectors() {
	front = glm::normalize(-position);
	right = glm::normalize(glm::cross(front, m_worldUp));
	up = glm::normalize(glm::cross(right, front));
}