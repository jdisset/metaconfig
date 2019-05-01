#pragma once

#include <boost/hana.hpp>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <regex>
#include "helper.hpp"
#include "includes/cxxopts.hpp"
#include "includes/json.hpp"

namespace std {
template <typename T, typename = typename T::isAConfigClass>
std::ostream& operator<<(std::ostream& os, const T& t) {
	os << t.print();
	return os;
}
inline std::ostream& operator<<(std::ostream& os, const std::vector<bool>& bf) {
	for (const auto& b : bf) os << b;
	return os;
}
inline std::istream& operator>>(std::istream& is, std::vector<bool>& bf) {
	bf.clear();
	char c;
	while (is.get(c)) bf.push_back(c == '0' ? false : true);
	return is;
}

}  // namespace std

inline constexpr bool is_associative_container(...) { return false; }

template <typename T> struct is_string { static const constexpr bool value = false; };
template <> struct is_string<std::string> { static const constexpr bool value = true; };

template <typename C, typename = typename C::mapped_type>
constexpr bool is_associative_container(C const*) {
	return !is_string<typename C::key_type>::value;
}
template <typename T> T fromStr(const std::string& str) {
	std::istringstream ss(str);
	T out;
	ss >> out;
	return out;
}

template <typename T> std::string toStr(const T& t) {
	std::ostringstream ss;
	ss << std::setprecision(std::numeric_limits<double>::max_digits10);
	ss << t;
	return ss.str();
}

// specialization for associative containers without string as a key type
namespace nlohmann {
template <typename T>
struct adl_serializer<
    T, typename std::enable_if<is_associative_container<T>(nullptr)>::type> {
	static void to_json(nlohmann::json& j, const T& m) {
		j = nlohmann::json();
		for (const auto& e : m) j[toStr(e.first)] = e.second;
	}

	static void from_json(const nlohmann::json& j, T& m) {
		m.clear();
		for (auto it = j.begin(); it != j.end(); ++it)
			m[fromStr<typename T::key_type>(it.key())] =
			    it.value().get<typename T::mapped_type>();
	}
};
}  // namespace nlohmann

template <class METACONFIG_T>
constexpr bool isConfig(typename METACONFIG_T::isAConfigClass*) {
	return true;
}
template <class METACONFIG_T> constexpr bool isConfig(...) { return false; }

class Argv {
 public:
	Argv(const std::vector<std::string>& args)
	    : m_argv(new char*[args.size()]), m_argc(args.size()) {
		int i = 0;
		auto iter = args.begin();
		while (iter != args.end()) {
			auto len = (*iter).size() + 1;
			auto ptr = std::unique_ptr<char[]>(new char[len]);
			std::strcpy(ptr.get(), (*iter).c_str());
			m_args.push_back(std::move(ptr));
			m_argv.get()[i] = m_args.back().get();
			++iter;
			++i;
		}
	}

	char** argv() const { return m_argv.get(); }

	int argc() const { return m_argc; }

 private:
	std::vector<std::unique_ptr<char[]>> m_args;
	std::unique_ptr<char*[]> m_argv;
	int m_argc;
};

template <typename T> struct Holder { T t; };
template <class T> std::ostream& operator<<(std::ostream& os, const Holder<T>& h) {
	nlohmann::json o(h.t);
	os << o.dump();
	return os;
}

/*template <typename T> std::string realTypeName() {*/
// int status;
// char* realname;
// realname = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
// free(realname);
// std::ostringstream rn;
// rn << realname;
// return rn.str();
/*}*/

#define METACONFIG_DECLARE_SERIALIZABLE_BODY(N)                                        \
	using isAConfigClass = bool;                                                         \
	void loadFromFile(std::string file) {                                                \
		std::ifstream t(file);                                                             \
		std::stringstream buffer;                                                          \
		buffer << t.rdbuf();                                                               \
		auto o = nlohmann::json::parse(buffer.str());                                      \
		from_json(o);                                                                      \
	}                                                                                    \
                                                                                       \
	void from_json(const nlohmann::json& js) {                                           \
		nlohmann::json o(js);                                                              \
		constexpr auto accessors = boost::hana::accessors<N>();                            \
		boost::hana::for_each(                                                             \
		    boost::hana::transform(accessors, [](auto a) { return a; }), [&](auto p) {     \
			    constexpr auto getMember = boost::hana::second(p);                           \
			    std::string k(boost::hana::to<char const*>(boost::hana::first(p)));          \
			    using m_t = std::remove_reference_t<decltype(getMember(*this))>;             \
			    if (o.count(k)) {                                                            \
				    /*std::cerr << "from_json: " << k << ", with type " << realTypeName<m_t>() \
				              << std::endl;   */                                               \
				    getMember(*this) = o.at(k).get<m_t>();                                     \
				    o.erase(k);                                                                \
			    }                                                                            \
		    });                                                                            \
		for (auto it = o.begin(); it != o.end(); ++it) {                                   \
			std::cerr << "Unknown param: " << it.key() << std::endl;                         \
		}                                                                                  \
	}                                                                                    \
                                                                                       \
	nlohmann::json to_json() const {                                                     \
		nlohmann::json j;                                                                  \
		boost::hana::for_each(*this, [&](auto p) {                                         \
			std::string k(boost::hana::to<char const*>(boost::hana::first(p)));              \
			j[k] = boost::hana::second(p);                                                   \
		});                                                                                \
		return j;                                                                          \
	}                                                                                    \
                                                                                       \
	std::string print() const {                                                          \
		std::stringstream out;                                                             \
		out << to_json().dump(2);                                                          \
		return out.str();                                                                  \
	}                                                                                    \
                                                                                       \
	void save(std::string file) const {                                                  \
		std::ofstream fs(file);                                                            \
		fs << print();                                                                     \
		fs.close();                                                                        \
	}                                                                                    \
                                                                                       \
	bool operator==(const N& n) const {                                                  \
		constexpr auto accessors = boost::hana::accessors<N>();                            \
		bool result = true;                                                                \
		boost::hana::for_each(boost::hana::transform(accessors, [](auto a) { return a; }), \
		                      [&](auto p) {                                                \
			                      constexpr auto getMember = boost::hana::second(p);         \
			                      if (getMember(*this) != getMember(n)) result = false;      \
		                      });                                                          \
		return result;                                                                     \
	}                                                                                    \
	bool operator!=(const N& n) const { return !(*this == n); }

// bool operator==(const N& n) const { return (to_json() == n.to_json()); }
// bool operator!=(const N& n) const { return !(*this == n); }

#define METACONFIG_DECLARE_PARSABLE(N)                                                \
	void parse(int argc, char** argv) {                                                 \
		cxxopts::Options opt(argv[0]);                                                    \
		declareOptions<N>("", *this, opt);                                                \
		opt.allow_unrecognised_options();                                                 \
		std::vector<std::string> strArgv;                                                 \
		for (int i = 0; i < argc; ++i) {                                                  \
			std::string s(argv[i]);                                                         \
			std::replace(s.begin(), s.end(), ':', '_');                                     \
			strArgv.push_back(s);                                                           \
		}                                                                                 \
		Argv margv(strArgv);                                                              \
		char** actualArgv = margv.argv();                                                 \
		opt.parse(argc, actualArgv);                                                      \
	}                                                                                   \
                                                                                      \
	template <typename C>                                                               \
	static inline void declareOptions(const std::string& nspace, C& c,                  \
	                                  cxxopts::Options& opt) {                          \
		constexpr auto accessors = boost::hana::accessors<C>();                           \
		boost::hana::for_each(                                                            \
		    boost::hana::transform(accessors, [](auto a) { return a; }), [&](auto p) {    \
			    std::string k(boost::hana::to<char const*>(boost::hana::first(p)));         \
			    k = nspace + k;                                                             \
			    auto& r = boost::hana::second(p)(c);                                        \
			    addOrRecurse(k, r, opt);                                                    \
		    });                                                                           \
	}                                                                                   \
                                                                                      \
	template <typename METACONFIG_T>                                                    \
	static inline typename std::enable_if<isConfig<METACONFIG_T>(nullptr)>::type        \
	    addOrRecurse(const std::string& name, METACONFIG_T& t, cxxopts::Options& opt) { \
		std::string nextnspace = name + "__";                                             \
		declareOptions<METACONFIG_T>(nextnspace, t, opt);                                 \
	}                                                                                   \
                                                                                      \
	template <typename METACONFIG_T>                                                    \
	static inline typename std::enable_if<!isConfig<METACONFIG_T>(nullptr)>::type       \
	    addOrRecurse(const std::string& name, METACONFIG_T& t, cxxopts::Options& opt) { \
		if (name[0] == '"' && name[name.size() - 1] == '"')                               \
			std::cerr << name << " is json" << std::endl;                                   \
		opt.add_options()(name, name,                                                     \
		                  cxxopts::value<std::remove_reference_t<decltype(t)>>(t));       \
	}

#define METACONFIG_DECLARE_CONFIG_BODY(N) \
	METACONFIG_DECLARE_SERIALIZABLE_BODY(N) \
	// METACONFIG_DECLARE_PARSABLE(N)

#define METACONFIG_DECLARE_SERIALIZABLE(N, ...) \
	BOOST_HANA_DEFINE_STRUCT(N, __VA_ARGS__);     \
	METACONFIG_DECLARE_SERIALIZABLE_BODY(N)

#define METACONFIG_DECLARE_SERIALIZABLE_WITH_DEFAULT(N, ...) \
	TUN_DEFINE_STRUCT(N, __VA_ARGS__);                         \
	METACONFIG_DECLARE_SERIALIZABLE_BODY(N)

#define METACONFIG_DECLARE_CONFIG(N, ...)   \
	BOOST_HANA_DEFINE_STRUCT(N, __VA_ARGS__); \
	METACONFIG_DECLARE_CONFIG_BODY(N)

#define METACONFIG_DECLARE(N, ...)   \
	TUN_DEFINE_STRUCT(N, __VA_ARGS__); \
	METACONFIG_DECLARE_CONFIG_BODY(N)

// TODO : generalize to any std container
#define METACONFIG_NAMESPACE_DEFINITIONS                                                 \
	template <typename T> void from_json(const nlohmann::json& j, T& e) {                  \
		e.from_json(j);                                                                      \
	}                                                                                      \
	template <typename T> void from_json(const nlohmann::json& j, std::vector<T>& e) {     \
		e = std::vector<T>();                                                                \
		for (const auto& i : j) {                                                            \
			T t;                                                                               \
			from_json(i, t);                                                                   \
			e.push_back(t);                                                                    \
		}                                                                                    \
	}                                                                                      \
	template <typename T, size_t N>                                                        \
	void from_json(const nlohmann::json& j, std::array<T, N>& e) {                         \
		for (size_t i = 0; i < N; ++i) {                                                     \
			T t;                                                                               \
			from_json(j[i], t);                                                                \
			e[i] = t;                                                                          \
		}                                                                                    \
	}                                                                                      \
	template <typename T> void to_json(nlohmann::json& j, const T& e) { j = e.to_json(); } \
	template <typename T> void to_json(nlohmann::json& j, const std::vector<T>& e) {       \
		j = nlohmann::json::array();                                                         \
		for (const auto& i : e) {                                                            \
			nlohmann::json o;                                                                  \
			to_json(o, i);                                                                     \
			j.push_back(o);                                                                    \
		}                                                                                    \
	}                                                                                      \
	template <typename T, size_t N>                                                        \
	void to_json(nlohmann::json& j, const std::array<T, N>& e) {                           \
		j = nlohmann::json::array();                                                         \
		for (const auto& i : e) {                                                            \
			nlohmann::json o;                                                                  \
			to_json(o, i);                                                                     \
			j.push_back(o);                                                                    \
		}                                                                                    \
	}
