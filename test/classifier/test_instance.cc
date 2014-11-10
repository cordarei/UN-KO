#include <catch/catch.hpp>

#include "../test_common.h"
#include "../../src/classifier/instance.h"


TEST_CASE("sentence_t can be default and copy constructed") {
  using foo::classifier::sentence_t;
  sentence_t i{};
  auto i2 = sentence_t{};
  auto i3{i2};
  auto i4 = i3;
}

TEST_CASE("sentence_t can be created from a JSON object") {
  auto j = get_json();
  auto &&k = j["sentences"][0];
  auto i = foo::classifier::make_sentence(k);

  REQUIRE(i.words == (std::vector<std::string>{"This", "is", "sentence", "one", "."}));
  REQUIRE((*i.answer).top_spans == (std::vector<foo::span_t>{
        {"NP", 0u, 1u}, {"VP", 1u, 4u}, {".", 4u, 5u}
      }));
  REQUIRE((*i.answer).legal_split_points == (std::vector<foo::offset_t>{
        1u, 4u
      }));
}
