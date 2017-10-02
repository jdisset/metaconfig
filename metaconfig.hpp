#ifndef METACONFIG_HPP
#define METACONFIG_HPP

#include <boost/hana.hpp>
#include <fstream>
#include "includes/json.hpp"

#define DECLARE_CONFIG(N, ...)                                                      \
	struct N {                                                                        \
		BOOST_HANA_DEFINE_STRUCT(N, ##__VA_ARGS__);                                     \
		template <class T> static constexpr auto f(T t) {                               \
			return std::make_pair(boost::hana::first(t), boost::hana::second(t));         \
		}                                                                               \
		void load(std::string file) {                                                   \
			std::ifstream t(file);                                                        \
			std::stringstream buffer;                                                     \
			buffer << t.rdbuf();                                                          \
			auto o = nlohmann::json::parse(buffer.str());                                 \
			constexpr auto accessors = boost::hana::accessors<MaConfig>();                \
			boost::hana::for_each(                                                        \
			    boost::hana::transform(accessors,                                         \
			                           f<decltype(accessors[boost::hana::size_c<0>])>),   \
			    [&](auto p) {                                                             \
				    constexpr auto getMember = p.second;                                    \
				    std::string k(boost::hana::to<char const*>(p.first));                   \
				    if (o.count(k)) {                                                       \
					    getMember(*this) = o.at(k);                                           \
					    o.erase(k);                                                           \
				    } else                                                                  \
					    std::cerr << "Unable to find param " << k << " in configuration file" \
					              << std::endl;                                               \
			    });                                                                       \
			for (auto it = o.begin(); it != o.end(); ++it) {                              \
				std::cerr << "Unknown param: " << it.key() << std::endl;                    \
			}                                                                             \
		}                                                                               \
                                                                                    \
		std::string print() {                                                           \
			nlohmann::json j;                                                             \
			std::stringstream out;                                                        \
			boost::hana::for_each(*this, [&](auto p) {                                    \
				std::string k(boost::hana::to<char const*>(boost::hana::first(p)));         \
				j[k] = boost::hana::second(p);                                              \
			});                                                                           \
			out << j.dump(2);                                                             \
			return out.str();                                                             \
		}                                                                               \
		void save(std::string file) {                                                   \
			std::ofstream fs(file);                                                       \
			fs << print();                                                                \
			fs.close();                                                                   \
		}                                                                               \
	};

#endif
