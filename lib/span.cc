#include "foo/span.h"

#include <ostream>

namespace foo {
  std::ostream & operator<<(std::ostream &out, span_t const &span) {
    out << span.label() << ":[" << span.begin() << "," << span.end() << "]";
    return out;
  }
}
