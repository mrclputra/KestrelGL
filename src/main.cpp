#include <iostream>
#include "logger.h"

int main() {
	logger.info("Hello CMake.");
	logger.info("C++ Standard ", __cplusplus);
	logger.info("KestrelGL2 ECS");
	logger.spacing();
	


	return 0;
}