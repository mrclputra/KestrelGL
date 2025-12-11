#include "Camera.h"

Camera::Camera(float radius, float theta, float pi, glm::vec3 worldUp ) {
	this->radius = radius;
	this->theta = theta;
	this->phi = pi;
	this->m_worldUp = worldUp;

	logger.info("camera created - " + std::to_string(m_viewportWidth) + "*" + std::to_string(m_viewportHeight));
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
	phi += yOffset * sensitivity * PIXEL_TO_RAD;

	// constraints
	if (phi > glm::radians(89.0f))
		phi = glm::radians(89.0f);
	if (phi < glm::radians(-89.0f))
		phi = glm::radians(-89.0f);

	update();
}
void Camera::zoom(float offset) {
	radius = glm::clamp(radius - offset * 0.32f, 0.2f, 700.0f);
	update();
}
void Camera::reset() {
	logger.info("camera reset");
	
	radius = 5.0f;
	theta = 0.0f;
	phi = 0.0f;
	update();
}

glm::vec3 Camera::calculatePosition() const {
	// spherical to cartesian
	return glm::vec3(
		radius * cos(phi) * cos(theta),
		radius * sin(phi),
		radius * cos(phi) * sin(theta)
	);
}

// https://www.songho.ca/opengl/gl_camera.html
void Camera::updateVectors() {
	front = glm::normalize(-position);
	right = glm::normalize(glm::cross(front, m_worldUp));
	up = glm::normalize(glm::cross(right, front));
}