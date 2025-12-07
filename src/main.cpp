#include <iostream>
#include "App.h"

int main() {
	logger.info("Hello CMake");
	logger.info("C++ Standard " + __cplusplus);
	logger.info("KestrelGL 2");

	// create an App instance
	// call the app run method 
	auto app = App(600, 600, "KestrelGL2");
	app.run();

	return 0;
}