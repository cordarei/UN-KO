#ifndef FOO_SRC_CLASSIFIER_CLASSIFIER_H
#define FOO_SRC_CLASSIFIER_CLASSIFIER_H

#include <foo/foo.h>
#include "../common.h"

namespace foo {
namespace classifier {

  struct instance {
    std::vector<word_t> words;
    std::vector<pos_t> tags;
    std::vector<span_t> top_spans;

    instance() = default;
    ~instance() = default;
    instance(instance const &) = default;
    instance(instance &&) = default;
    instance & operator=(instance const &) = default;
    instance & operator=(instance &&) = default;
  };

  int train();
  int test();
  int run();
}
}

#endif
