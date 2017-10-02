#include <iostream>
#include "../metaconfig.hpp"

DECLARE_CONFIG(MaConfig, (int, lol));

int main(int argc, char** argv) {
	MaConfig cfg;
	cfg.load("test.cfg");
	std::cout << "Cgf lol = " << cfg.lol << std::endl;
	cfg.lol = 8;
	cfg.save("saved.cfg");
};
