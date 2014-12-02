#ifndef FOO_SRC_PARSER_PARSER_H
#define FOO_SRC_PARSER_PARSER_H


#include "../common.h"

namespace foo {
namespace parser {

  int extract_grammar(docopt_t const &args);
  int run(docopt_t const &args);

}
}

#endif
