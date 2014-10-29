#ifndef FOO_INCLUDE_FOO_H
#define FOO_INCLUDE_FOO_H

namespace foo {

  using word_t = std::string;
  using phrase_t = std::string;
  using pos_t = std::string;
  using symbol_t = std::string; // pos_t | phrase_t
  using offset_t = size_t;

  class span_t {
  public:
  span_t(symbol_t l, offset_t b, offset_t e)
    : _label{l}, _begin{b}, _end{e}
    {}

    symbol_t const & label() const { return _label; }
    offset_t begin() const { return _begin; }
    offset_t end() const { return _end; }

  private:
    symbol_t _label;
    offset_t _begin;
    offset_t _end;
  };

  std::ostream & operator<<(std::ostream &out, span_t const &span) {
    out << span.label() << ":[" << span.begin() << "," << span.end() << "]";
    return out;
  }

}

#endif
