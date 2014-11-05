#include <catch/catch.hpp>

#include <string>
#include <vector>


SCENARIO("a feature registry can be created and used" "[features]") {
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
  auto feature_reg = feature_registry_t<std::vector<std::string>>{};
  feature_reg.add_feature(bigrams);

  WHEN("the example function is used") {
    auto bs = bigrams(tags);
    THEN("it returns all the bigrams") {
      REQUIRE(bigrams(tags) == (std::vector<bigram_t>{{"^","A"}, {"A", "B"}, {"B", "C"}, {"C", "$"}}));
    }
  }

}
