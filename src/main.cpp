#include <iostream>
#include "logger.h"

#include "App.hpp"

int main() {
	logger.info("Hello CMake.");
	logger.info("C++ Standard ", __cplusplus);
	logger.info("KestrelGL2 ECS");
	logger.spacing();
	
	// App wrapper class can be retained from the old version
	// I don't see any reason why I should make it anything else (RAII)
	auto app = App(640, 480, "KestrelGL2 ECS");
	app.run();

	return 0;
}