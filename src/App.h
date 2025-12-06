#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

#include <memory>
#include <vector>
#include <string>

#include "Scene.h"
#include "eventbus.h"

class App {
public:
    App(int width, int height, const char* title);
    ~App();
    
    void run(); // execution loop
    
    // callback handlers
    void onFrameBufferSize(int width, int height);
    void onCursorPos(double xPos, double yPos);
    void onScroll(double xOff, double yOff);
    void onKey(int key, int scancode, int action, int mods);

    bool firstMouse = true;
    bool mousePressed = false;
    
    std::shared_ptr<Scene> scene;
    EventBus bus;

private:
    void init();
    void setupCallbacks();
    void cleanup();

    GLFWwindow* window = nullptr;
    int width, height;
    const char* title;

    // cursor position
    float lastX = 0.0f;
    float lastY = 0.0f;
};