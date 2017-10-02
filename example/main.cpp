#include <iostream>
#include "../metaconfig.hpp"

DECLARE_CONFIG(MyConfig, (int, varA), (double, varB),
               (std::string, varC));  // declaring the config class with all the params

int main(int argc, char** argv) {
	MyConfig cfg;
	cfg.load("test.cfg");   // load from a file
	cfg.parse(argc, argv);  // parse command line
	cfg.save("saved.cfg");  // save the current config
};
