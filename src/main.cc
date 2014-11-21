#include <iostream>

#include "common.h"
#include "classifier/classifier.h"
#include "parser/parser.h"


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


int main(int argc, const char** argv) {
  using namespace foo;

  auto args = docopt::docopt(USAGE,
                             { argv + 1, argv + argc },
                             true,
                             VERSION);

  if (check_docopt_command(args, {"classifier", "train"})) {
    std::cout << "training classifier" << std::endl;
    return classifier::train(args);
  }
  if (check_docopt_command(args, {"classifier", "test"})) {
    std::cout << "testing classifier" << std::endl;
    return classifier::test(args);
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
