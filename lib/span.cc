#include "foo/span.h"

#include <ostream>

namespace foo {
  bool operator==(span_t const &l, span_t const &r) {
    return l.label() == r.label() && l.begin() == r.begin() && l.end() == r.end();
  }

  std::ostream & operator<<(std::ostream &out, span_t const &span) {
    out << span.label() << ":[" << span.begin() << "," << span.end() << "]";
    return out;
  }
}
