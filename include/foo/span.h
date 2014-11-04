#ifndef FOO_INCLUDE_SPAN_H
#define FOO_INCLUDE_SPAN_H


#include "types.h"
#include <iosfwd>

namespace foo {
  class span_t {
  private:
    symbol_t label_;
    offset_t begin_;
    offset_t end_;

  public:
    span_t(symbol_t l, offset_t b, offset_t e)
      : label_{l}
      , begin_{b}
      , end_{e}
    {}

    span_t() = delete;
    ~span_t() = default;
    span_t(span_t const &) = default;
    span_t(span_t &&) = default;
    span_t & operator=(span_t const &) = default;
    span_t & operator=(span_t &&) = default;

    symbol_t const & label() const { return label_; }
    offset_t begin() const { return begin_; }
    offset_t end() const { return end_; }
  };

  std::ostream & operator<<(std::ostream &out, span_t const &span);
}


#endif
