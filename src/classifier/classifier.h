#ifndef FOO_SRC_CLASSIFIER_CLASSIFIER_H
#define FOO_SRC_CLASSIFIER_CLASSIFIER_H

#include "../common.h"

namespace foo {
namespace classifier {

  int train(docopt_t const &args);
  int test(docopt_t const &args);
  int run(docopt_t const &args);

}
}

#endif
