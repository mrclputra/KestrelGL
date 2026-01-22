#pragma once

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include "logger.h"

#include "Gui.hpp"

class App {
public:
	App(int width, int heigt, const char* title);
	~App();

	void run();

	// callback handlers
	void onFrameBufferSize(int width, int height);
	void onKey(int key, int scancode, int action, int mods);
	void onCursorPos(double xPos, double yPos);
	void onScroll(double xOff, double yOff);

	void processInput(float dt);

	bool firstMouse = true;
	bool mousePressed = false;
	float lastX = 0.0f;
	float lastY = 0.0f;

private:
	void init();
	void setupCallbacks();
	void cleanup();

	GLFWwindow* window = nullptr;
	int width, height;
	const char* title;

	Gui gui;
};