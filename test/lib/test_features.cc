#include <catch/catch.hpp>

#include <string>
#include <vector>


template<typename T>
using unigram_t = T;
template<typename T>
using bigram_t = std::pair<T, T>;
template<typename T>
using trigram_t = std::tuple<T, T, T>;

struct pos_bigram_feature_tag;

TEST_CASE("we can define a feature generator") {
  using bigram_t = std::pair<std::string, std::string>;
  auto tags = std::vector<std::string>{"A", "B", "C"};
  auto bigrams = [](auto &&ts) {
    auto bs = std::vector<bigram_t>{};
    auto bos = std::string{"^"};
    auto const * last = &bos;
    for(auto const& t : ts) {
      bs.emplace_back(*last, t);
      last = &t;
    }
    bs.emplace_back(*last, "$");
    return bs;
  };

  REQUIRE(bigrams(tags) == (std::vector<bigram_t>{{"^","A"}, {"A", "B"}, {"B", "C"}, {"C", "$"}}));

}
