#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuiFileDialog.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class App; // forward declaration

class Gui {
public:
    Gui();
    ~Gui();

    void init(App* appPtr, GLFWwindow* window);
    void shutdown();

    void beginFrame();
    void endFrame();

    void draw();

private:
    App* app = nullptr; // to interact with application processes
    bool active = false; // flag
};