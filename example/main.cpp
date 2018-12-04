#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "../metaconfig.hpp"

struct Seri {
	DECLARE_SERIALIZABLE(Seri, (std::string, argS), (std::vector<double>, vd),
	                     (std::map<size_t, double>, umap),
	                     (std::map<std::vector<bool>, std::vector<bool>>, bmap))
	Seri() {
		argS = "bonjourSeri";
		vd = {{0.1, 0.5}};
		umap = {{{1, 50.123}, {25, 70.05}}};
		bmap = {{{true, true, false}, {false}}};
	}
};

struct Config {
	struct Nested {
		DECLARE_CONFIG(Nested, (std::string, var), (std::vector<double>, vec), (Seri, seri))
		Nested() {
			var = "hello";
			vec = std::vector<double>({0, 1, 2});
		}
	};

	DECLARE_CONFIG_WITH_DEFAULT(Config, (int, a, 5), (Nested, n, ))  //, (Nested, nested));
	Config() {}
};

template <typename T> void from_json(const nlohmann::json& j, T& e) { e.from_json(j); }
template <typename T> void to_json(nlohmann::json& j, const T& e) { j = e.to_json(); }

int main(int argc, char** argv) {
	Config cfg;
	// cfg.parse(argc, argv);
	std::cout << cfg.print() << std::endl;  // save the current config
	Config cfg2;
	cfg.n.seri.argS = "lolilol";
	cfg2.from_json(cfg.to_json());
	assert(cfg == cfg2);
	std::cerr << cfg2.n.seri.argS << std::endl;

	return 0;
}
