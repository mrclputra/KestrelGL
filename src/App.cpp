#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "App.h"

#include "debug.h"

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

	// lmb
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			double xPos, yPos;
			glfwGetCursorPos(window, &xPos, &yPos);
			app->lastX = xPos;
			app->lastY = yPos;

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
		logger.error("Failed to initialize GLAD");
		std::exit(EXIT_FAILURE);
	}

	// TODO: find a way to allow the logger to also parse C-string literals
	logger.info("OpenGL Version: " + std::string((const char*)glGetString(GL_VERSION)));
	logger.info("GLSL Version: " + std::string((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)));

	//std::cout << "\n";
	//std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";
	//std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

	// initialize ImGui
	gui.init(this, window);

	// initialize viewport
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	glViewport(0, 0, fbWidth, fbHeight);

	// setup OpenGL configurations here
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);*/
	//glCullFace(GL_FRONT);

	// initialize and configure scene instance
	scene = std::make_unique<Scene>(bus);
	scene->camera.setViewport(fbWidth, fbHeight); // tell camera about viewport

	// open debug scene
	debugScene(*scene);

	logger.info("ended initialization");
}

void App::setupCallbacks() {
	// the problem is that we store our callback functions in C++ classes, but glfw requires C functions
	// workaround is to format as C functions that call into the C++ classes
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
}

void App::run() {
	logger.info("running main process...");
	// main stuff happens in this function here
	float lastTime = 0.0f;

	// main execution loop below
	// per-frame logic
	while (!glfwWindowShouldClose(window)) {
		float currentTime = glfwGetTime();
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// do stuff here

		// check if shader files modified? (hot reload)
		// TODO: implement and call here

		// clear render buffers
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw gui
		gui.beginFrame();
		gui.draw();

		// render and update scene
		scene->update(deltaTime);
		scene->render();

		// end frame
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

// callbacks
void App::onFrameBufferSize(int w, int h) {
	glViewport(0, 0, w, h);
	width = w;
	height = h;

	// update camera viewport
	if (scene) {
		scene->camera.setViewport(w, h);
	}
}

void App::onCursorPos(double xPos, double yPos) {
	if (!mousePressed) return;

	// prevent jumping, set this position to initial
	// as such, the offsets are 0
	if (firstMouse) {
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
		return; // skip frame
	}

	float xOffset = xPos - lastX;
	float yOffset = yPos - lastY;
	lastX = xPos;
	lastY = yPos;

	// TODO: camera.rotate
	scene->camera.rotate(xOffset, yOffset);

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
	scene->camera.zoom(yOff);
}

void App::onKey(int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}