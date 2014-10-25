#include <algorithm>
#include <iostream>
#include <iterator>


constexpr auto USAGE =
u8R"(  foo

  Usage:
    foo classifier train
    foo classifier run
    foo classifier test
    foo parser extract-grammar
    foo parser run
)";
constexpr auto VERSION = u8"foo 0.0.0";


#include <docopt/docopt.h>
using docopt_t = decltype(docopt::docopt({}, {}));
using docopt::get;

bool check_docopt_flag(docopt_t const &opt, std::string const &flag) {
  return opt.at(flag).isBool() && get<bool>(opt.at(flag));
}

/*
 * `opt`: the std::map returned from docopt::docopt() or docopt::docopt_parse()
 * `commands`: list of <command>, <sub-command>, <sub-sub-command>, etc
 *
 * Returns: true if the given commands matches the command parsed by docopt, false otherwise
 */
bool check_docopt_command(docopt_t const &opt, std::vector<std::string> commands) {
  return std::all_of(std::begin(commands),
                     std::end(commands),
                     [&](auto cmd) { return check_docopt_flag(opt, cmd); });
}


#include "classifier/classifier.h"
#include "parser/parser.h"

int main(int argc, const char** argv) {
  auto args = docopt::docopt(USAGE,
                             { argv + 1, argv + argc },
                             true,
                             VERSION);

  if (check_docopt_command(args, {"classifier", "train"})) {
    std::cout << "training classifier" << std::endl;
    return classifier::train();
  }
  if (check_docopt_command(args, {"classifier", "test"})) {
    std::cout << "testing classifier" << std::endl;
    return classifier::test();
  }
  if (check_docopt_command(args, {"classifier", "run"})) {
    std::cout << "running classifier" << std::endl;
    return classifier::run();
  }

  if (check_docopt_command(args, {"parser", "extract-grammar"})) {
    std::cout << "extracting grammar with parser" << std::endl;
    return parser::extract_grammar();
  }
  if (check_docopt_command(args, {"parser", "run"})) {
    std::cout << "running parser" << std::endl;
    return parser::run();
  }
}
