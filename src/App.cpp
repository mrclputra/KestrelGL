#define STB_IMAGE_IMPLEMENTATION
#include "App.h"

// glfw callbacks
static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	auto app = static_cast<App*>(glfwGetWindowUserPointer(window)); // is there a way to get this value without casting each time?
	app->onFrameBufferSize(width, height);
}
static void cursor_position_callback(GLFWwindow* window, double xPos, double yPos) {
	auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
	app->onCursorPos(xPos, yPos);
}
static void scroll_callback(GLFWwindow* window, double xOff, double yOff) {
	auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
	app->onScroll(xOff, yOff);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
	app->onKey(key, scancode, action, mods);
}
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	auto app = static_cast<App*>(glfwGetWindowUserPointer(window));

	// lmb
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			app->mousePressed = true;
			app->firstMouse = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // hide cursor
		}
		else if (action == GLFW_RELEASE) {
			app->mousePressed = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}

// app constructor
App::App(int w, int h, const char* t) 
	: window(nullptr), width(w), height(h), title(t) {
	init();
}
App::~App() {}

void App::init() {
	// initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW\n";
		std::exit(EXIT_FAILURE);
	}

	// configure GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window
	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (window == NULL) {
		std::cerr << "Failed to create a GLFW window\n";
		glfwTerminate();
		std::exit(EXIT_FAILURE);
	}

	// center window
	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primary);
	int xpos = (mode->width - width) / 2;
	int ypos = (mode->height - height) / 2;
	glfwSetWindowPos(window, xpos, ypos);

	glfwMakeContextCurrent(window);
	glfwSetWindowUserPointer(window, this);

	// setup callbacks
	setupCallbacks();

	// cursor config
	// TOOD: review; shouldnt this already be handled by one of the callbacks above?
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // normal mode

	// load GLAD OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
		std::exit(EXIT_FAILURE);
	}

	// initialize ImGui
	// TODO: configure imgui instance

	// initialize viewport
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	glViewport(0, 0, fbWidth, fbHeight);

	// setup OpenGL configurations here
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// initialize or call scene here
	// TODO: configure scene instance
}

void App::setupCallbacks() {
	// the problem is that we store our callback functions in C++ classes, but glfw requires C functions
	// workaround is to format as C functions that call into the C++ classes
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
}

void App::run() {
	// main stuff happens in this function here
	float lastTime = 0.0f;

	// main execution loop below
	// per-frame logic
	while (!glfwWindowShouldClose(window)) {
		float currentTime = glfwGetTime();
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// do stuff here

		// check shaders (hot reload)

		// draw gui

		// render scene
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// aka Scene.run() or something
		// need to pass in the time values from above

		// end frame
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
		float lastX = xPos;
		float lastY = yPos;
		firstMouse = false;
	}

	float xOffset = xPos - lastX;
	float yOffset = yPos - lastY;
	lastX = xPos;
	lastY = yPos;

	// TODO: camera.rotate

	// wrap cursor
	int w, h;
	glfwGetWindowSize(window, &w, &h);

	bool wrapped = false;
	if (xPos <= 0) { xPos = w - 2; wrapped = true; }
	if (xPos >= w - 1) { xPos = 1; wrapped = true; }
	if (yPos <= 0) { yPos = h - 2; wrapped = true; }
	if (yPos >= h - 1) { yPos = 1; wrapped = true; }

	if (wrapped) {
		glfwSetCursorPos(window, xPos, yPos);
		lastX = xPos;
		lastY = yPos;
	}
}

void App::onScroll(double xOff, double yOff) {
	// TODO: camera.zoom(yOff);
}

void App::onKey(int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}