#include <iostream>
#include "../metaconfig.hpp"
DECLARE_CONFIG(MaConfig, (int, lol), (std::string, bite));

int main(int argc, char** argv) {
	MaConfig cfg;
	cfg.load("test.cfg");
	cfg.parse(argc, argv);
	std::cout << "Cgf lol = " << cfg.lol << std::endl;
	cfg.lol = 8;
	cfg.save("saved.cfg");
};
