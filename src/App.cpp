#include "App.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <chrono>

// glfw callbacks
static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	auto app = static_cast<App*>(glfwGetWindowUserPointer(window)); // is there a way to get this value without casting each time?
	app->onFrameBufferSize(width, height);
}
static void cursor_position_callback(GLFWwindow* window, double xPos, double yPos) {
	ImGui_ImplGlfw_CursorPosCallback(window, xPos, yPos); // ImGui
	if (ImGui::GetIO().WantCaptureMouse) return; // if imgui wants mouse position, stop here

	auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
	app->onCursorPos(xPos, yPos);
}
static void scroll_callback(GLFWwindow* window, double xOff, double yOff) {
	ImGui_ImplGlfw_ScrollCallback(window, xOff, yOff); // ImGui
	if (ImGui::GetIO().WantCaptureMouse) return; // if imgui wants the scroll, stop here

	auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
	app->onScroll(xOff, yOff);
}
static void character_callback(GLFWwindow* window, unsigned int codepoint) {
	ImGui_ImplGlfw_CharCallback(window, codepoint);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods); // ImGui
	if (ImGui::GetIO().WantCaptureKeyboard) return; // if imgui wants the keyboard, stop here

	auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
	app->onKey(key, scancode, action, mods);
}
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods); // ImGui
	if (ImGui::GetIO().WantCaptureMouse) return; // if imgui wants mouse button control, stop here

	auto app = static_cast<App*>(glfwGetWindowUserPointer(window));

	// mouse buttons
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			double xPos, yPos;
			glfwGetCursorPos(window, &xPos, &yPos);
			app->lastX = xPos;
			app->lastY = yPos;

			app->mousePressed = true;
			app->firstMouse = true;

			// hide cursor
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else if (action == GLFW_RELEASE) {
			app->mousePressed = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}

void App::setupCallbacks() {
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCharCallback(window, character_callback);
}

App::App(int width, int height, const char* title) {
	this->width = width;
	this->height = height;
	this->title = title;
	init();
}
App::~App() {}

void App::init() {
	// configure GLFW
	if (!glfwInit()) {
		logger.error("Failed to initialize GLFW");
		std::exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // no legacy

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (window == NULL) {
		std::cerr << "Failed to create a GLFW window\n";
		glfwTerminate();
		std::exit(EXIT_FAILURE);
	}

	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primary);
	int xpos = (mode->width - width) / 2;
	int ypos = (mode->height - height) / 2;
	glfwSetWindowPos(window, xpos, ypos);

	glfwMakeContextCurrent(window);
	glfwSetWindowUserPointer(window, this);

	// vsync disabled
	glfwSwapInterval(0);

	setupCallbacks();
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// load GLAD Opengl function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		logger.error("Failed to initialize GLAD");
		std::exit(EXIT_FAILURE);
	}

	logger.info("OpenGL Version: ", glGetString(GL_VERSION));
	logger.info("GLSL Version: ", glGetString(GL_SHADING_LANGUAGE_VERSION));

	gui.init(window);

	// viewport
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	glViewport(0, 0, fbWidth, fbHeight);

	// configure OpenGL
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// TODO: start ECS systems here

	// TODO: open a debug scene
	auto start = std::chrono::high_resolution_clock::now();
	double value = 1.0;
	for (int i = 0; i < 10000; i++) {
		value = std::sqrt(value + i);
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration = end - start;

	logger.info("Scene loaded in ", duration.count(), "ms");
	logger.info("Ended initialization");
}

void App::run() {
	logger.info("running main process...");
	float lastTime = 0.0f;

	// main execution is here, per-frame logic
	while (!glfwWindowShouldClose(window)) {
		float currentTime = glfwGetTime();
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		processInput(deltaTime);

		// clear render buffers
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// do stuff here
		gui.beginFrame();
		gui.draw();
		gui.endFrame();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	cleanup();
}

void App::cleanup() {
	if (window) {
		glfwDestroyWindow(window);
		window = nullptr;
	}
	glfwTerminate();
}


void App::processInput(float dt) {
	// process continuous input
	// this is needed for non-discrete functionality that depend on deltatime
	
}

// callbacks
void App::onFrameBufferSize(int w, int h) {
	glViewport(0, 0, w, h);
	width = w;
	height = h;
}

void App::onCursorPos(double xPos, double yPos) {
	if (!mousePressed) return;

	// prevent jumping, set this position to initial
	// as such, the offsets are 0
	if (firstMouse) {
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
		return;
	}

	float xOffset = xPos - lastX;
	float yOffset = yPos - lastY;
	lastX = xPos;
	lastY = yPos;
}

void App::onScroll(double xOff, double yOff) {
	// do this on scroll
}

void App::onKey(int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}