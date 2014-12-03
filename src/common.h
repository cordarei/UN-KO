#ifndef FOO_SRC_COMMON_H
#define FOO_SRC_COMMON_H


#include <iterator>
#include <algorithm>
#include <iostream>
#include <experimental/optional>

#include <docopt/docopt.h>
#include <json11/json11.hpp>
#include <range/v3/lines_range.hpp>
#include <foo/utility/join.h>


namespace {
  using docopt_t = decltype(docopt::docopt({}, {}));

  inline
  bool check_docopt_flag(docopt_t const &opt, std::string const &flag) {
    using docopt::get;
    return opt.at(flag).isBool() && get<bool>(opt.at(flag));
  }

  /*
   * `opt`: the std::map returned from docopt::docopt() or docopt::docopt_parse()
   * `commands`: list of <command>, <sub-command>, <sub-sub-command>, etc
   *
   * Returns: true if the given commands matches the command parsed by docopt, false otherwise
   */
  inline
  bool check_docopt_command(docopt_t const &opt, std::vector<std::string> commands) {
    return std::all_of(std::begin(commands),
                       std::end(commands),
                       [&](auto cmd) { return check_docopt_flag(opt, cmd); });
  }

  template<typename T>
  std::experimental::optional<T> check_docopt_arg(docopt_t const &opt, std::string const &arg) {
    auto it = opt.find(arg);
    if (it != opt.end()) {
      if (docopt::is<T>(it->second)) {
        return docopt::get<T>(it->second);
      }
    }

    return std::experimental::nullopt;
  }


  using json = json11::Json;

  inline
  std::string read_all(std::istream &is) {
    return foo::join(ranges::lines(is));
  }

  inline
  json read_json(std::istream& is) {
    std::string err;
    return json::parse(read_all(is), err);
  }

  inline
  size_t subclamp(size_t left, size_t right) {
    if (left < right) {
      return 0;
    } else {
      return left - right;
    }
  }

  template <typename S>
  std::string concat(S && s) {
    auto result = std::string{std::forward<S>(s)};
    return result;
  }

  template <typename S1, typename S2, typename ...Ss>
  std::string concat(S1 && s1, S2 && s2, Ss &&... rest) {
    auto result = std::string{std::forward<S1>(s1)};
    result += std::forward<S2>(s2);
    if (0 < sizeof...(Ss)) {
      result = concat(std::move(result), std::forward<Ss>(rest)...);
    }
    return result;
  }


  struct tracer_t {
    using literal_t = char const * const;
    literal_t file;
    literal_t function;
    int line;
    tracer_t(literal_t fi, literal_t fun, int ln) : file{fi}, function{fun}, line{ln} {
      std::cerr << " (" << file << "::" << line << " " << function << " -- enter.) " << std::endl;
    }
    ~tracer_t() {
      std::cerr << " (" << file << "::" << line << " " << function << " -- leave.) " << std::endl;
    }
  };

  //#define VERBOSE

#ifdef VERBOSE
#define log(x) std::cerr << __FILE__ << "::" << __LINE__ << " " << __PRETTY_FUNCTION__ << " -- " << x << std::endl
#define trace() tracer_t trace_42{__FILE__, __PRETTY_FUNCTION__, __LINE__}
#else
#define log(x)
#define trace()
#endif
}


#endif
