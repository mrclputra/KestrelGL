#include "Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 worldUp)
	: position(pos), m_worldUp(worldUp) {
	logger.info("camera created - " + std::to_string(m_viewportWidth) + "*" + std::to_string(m_viewportHeight));
	updateVectors();
}

glm::mat4 Camera::getViewMatrix() const {
	return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix() const {
	return glm::perspective(
		glm::radians(fov),
		(float)m_viewportWidth / (float)m_viewportHeight,
		0.1f,
		100.0f
	);
}

void Camera::rotate(float xOffset, float yOffset) {
	yaw += xOffset * sensitivity;
	pitch -= yOffset * sensitivity;

	pitch = glm::clamp(pitch, -89.0f, 89.0f);

	updateVectors();
}

void Camera::updateVectors() {
	glm::vec3 f;
	f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	f.y = sin(glm::radians(pitch));
	f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	// https://www.songho.ca/opengl/gl_camera.html
	front = glm::normalize(f);
	right = glm::normalize(glm::cross(front, m_worldUp));
	up = glm::normalize(glm::cross(right, front));
}

void Camera::moveForward(float dt) { position += front * speed * dt; }
void Camera::moveBackward(float dt) { position -= front * speed * dt; }
void Camera::moveRight(float dt) { position += right * speed * dt; }
void Camera::moveLeft(float dt) { position -= right * speed * dt; }
void Camera::moveUp(float dt) { position += m_worldUp * speed * dt; }
void Camera::moveDown(float dt) { position -= m_worldUp * speed * dt; }

void Camera::setViewport(int width, int height) {
	m_viewportWidth = width;
	m_viewportHeight = height;
}