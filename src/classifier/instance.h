#ifndef FOO_SRC_CLASSIFIER_INSTANCE_H
#define FOO_SRC_CLASSIFIER_INSTANCE_H


#include <vector>
#include <foo/types.h>
#include <foo/span.h>


namespace json11 { class Json; }
using json = json11::Json;


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

  instance make_instance(json const &j);

}
}


#endif
