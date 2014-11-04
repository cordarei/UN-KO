#ifndef FOO_SRC_COMMON_H
#define FOO_SRC_COMMON_H


#include <iterator>
#include <algorithm>

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
}


#endif
