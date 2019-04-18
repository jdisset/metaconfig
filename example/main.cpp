#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "../metaconfig.hpp"

METACONFIG_NAMESPACE_DEFINITIONS

struct Seri {
	METACONFIG_DECLARE_SERIALIZABLE(Seri, (std::string, argS))
};

struct Config {
	struct Nested {
		METACONFIG_DECLARE(Nested, (std::string, var, "default Nested"), (Seri, s,))
	};
	METACONFIG_DECLARE(Config, (int, a, 7), (std::vector<Nested>, n, ))
};

int main(int argc, char** argv) {
	Config cfg;
	cfg.n.push_back(Config::Nested());
	cfg.n.push_back(Config::Nested());
	std::cout << cfg.to_json().dump(2) << std::endl;
	return 0;
}
