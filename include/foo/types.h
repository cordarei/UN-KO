#ifndef FOO_INCLUDE_TYPES_H
#define FOO_INCLUDE_TYPES_H


#include <cstddef>
#include <string>

namespace foo {

  using word_t = std::string;
  using phrase_t = std::string;
  using pos_t = std::string;
  using symbol_t = std::string; // pos_t | phrase_t
  using offset_t = size_t;

}


#endif
