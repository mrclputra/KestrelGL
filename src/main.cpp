#include <iostream>
#include "App.h"

int main() {
	logger.info("Hello CMake");
	logger.info("C++ Standard " + std::to_string(__cplusplus));
	logger.info("KestrelGL2");
	logger.spacing();

	// create an App instance
	// call the app run method 
	auto app = App(600, 1000, "KestrelGL2");
	app.run();

	return 0;
}