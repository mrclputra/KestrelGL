#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuiFileDialog.h>

class App;

class Gui {
public:
	Gui() = default;
	~Gui() = default;

	void init(GLFWwindow* window);
	void shutdown();

	void beginFrame();
	void endFrame();

	void draw();
};