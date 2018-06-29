#include <iostream>
#include "../metaconfig.hpp"

struct Config {
	struct Nested {
		DECLARE_CONFIG(Nested, (std::string, var), (std::vector<double>, vec));
		Nested() {
			var = "hello";
			vec = std::vector<double>({0, 1, 2});
		}
	};

	DECLARE_CONFIG(Config, (int, a), (Nested, nested));
	Config() { a = 2; }
};

template <typename T> void from_json(const nlohmann::json& j, T& e) { e.from_json(j); }
template <typename T> void to_json(nlohmann::json& j, const T& e) { j = e.to_json(); }

int main(int argc, char** argv) {
	Config cfg;
	std::cout << cfg.print() << std::endl;  // save the current config
	return 0;
}
