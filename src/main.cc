#include <iostream>

#include "common.h"
#include "classifier/classifier.h"
#include "parser/parser.h"


constexpr auto USAGE =
u8R"(  foo

  Usage:
    foo classifier train [options]
    foo classifier test [options]
    foo parser extract-grammar
    foo parser run [options]

  Options:
    --update=<upd>     Classifier update [default: binary]
    --input=<in>       Input file
    --model=<model>    Model name (for feature and weights files) [default: model]
    --output=<out>     Output file
    --feat-pos         POS features
    --feat-word        Word features
    --feat-global      Global features
    --feat-local       Local Features
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
