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

  template <typename T>
  using unigram_t = std::tuple<T>;
  template <typename T>
  using bigram_t = std::tuple<T, T>;
  template <typename T>
  using trigram_t = std::tuple<T, T, T>;
}


#endif
