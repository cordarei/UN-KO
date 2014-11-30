#ifndef FOO_SRC_CLASSIFIER_INSTANCE_H
#define FOO_SRC_CLASSIFIER_INSTANCE_H


#include <vector>
#include <experimental/optional>
#include <foo/types.h>
#include <foo/span.h>

#include "config.h"


namespace json11 { class Json; }
using json = json11::Json;


namespace foo {
namespace classifier {

  struct answer_t {
    std::vector<span_t> top_spans;
    std::vector<offset_t> legal_split_points;
  };

  struct sentence_t {
    size_t index;
    std::vector<word_t> words;
    std::vector<pos_t> tags;
    std::experimental::optional<answer_t> answer;
  };

  sentence_t make_sentence(json const &j);


  struct structure_cache_t {
    std::vector<bigram_t<pos_t>> pos_bigrams;
    std::vector<trigram_t<pos_t>> pos_trigrams;

    std::vector<bigram_t<word_t>> word_bigrams;
    std::vector<trigram_t<word_t>> word_trigrams;

    structure_cache_t() = default;
    ~structure_cache_t() = default;
    structure_cache_t(structure_cache_t const &) = delete;
    structure_cache_t(structure_cache_t &&) = default;
    structure_cache_t & operator=(structure_cache_t const &) = delete;
    structure_cache_t & operator=(structure_cache_t &&) = default;
  };


  struct instance_t {
  private:
    sentence_t const * sentence_;
    structure_cache_t mutable * cache_;
    offset_t sp_;

  public:
    instance_t(sentence_t const &sentence, structure_cache_t &cache, offset_t sp)
      : sentence_{&sentence}, cache_{&cache}, sp_{sp}
    {}

    instance_t() : sentence_{nullptr}, cache_{nullptr}, sp_{static_cast<offset_t>(-1)} {}

    sentence_t const & sentence() const { return *sentence_; }
    structure_cache_t & cache() const { return *cache_; }
    offset_t sp() const { return sp_; }
  };

  bool is_legal(instance_t const &);

}
}


#endif
