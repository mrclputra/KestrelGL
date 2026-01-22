#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuiFileDialog.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

class App; // forward declaration

class Gui {
public:
    Gui() = default;
    ~Gui() = default;

    void init(App* appPtr, GLFWwindow* window);
    void shutdown();

    void beginFrame();
    void endFrame();

    void draw();

private:
    App* app = nullptr; // to interact with application processes
    bool active = false; // flag

    // profiler stuff
    static const int FRAME_HIST_COUNT = 100;
    float frameTimeHistory[FRAME_HIST_COUNT] = { 0 };
    int frameTimeOffset = 0;
    float maxFrameTime = 0.0f;

    float frameTimeSum = 0.0f;     // running sum; moving average
    float movingAverage = 0.0f;
};