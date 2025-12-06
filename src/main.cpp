#include <iostream>
#include "App.h"

int main() {
  std::cout << "Hello CMake\n";
  std::cout << "C++ Standard " << __cplusplus << "\n";
  std::cout << "KestrelGL2\n";

  // create an App instance
  // call the app run method 
  auto app = App(600, 600, "KestrelGL2");
  app.run();

  return 0;
}