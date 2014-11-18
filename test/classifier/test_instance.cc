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

TEST_CASE("is_legal returns true for legal instances") {
  auto j = get_json();
  auto &&k = j["sentences"][0];
  auto sent = foo::classifier::make_sentence(k);
  auto c = foo::classifier::structure_cache_t{};

  auto i1 = foo::classifier::instance_t{sent, c, 1};
  REQUIRE(foo::classifier::is_legal(i1) == true);
  auto i2 = foo::classifier::instance_t{sent, c, 4};
  REQUIRE(foo::classifier::is_legal(i2) == true);
}

TEST_CASE("is_legal returns false for illegal instances") {
  auto j = get_json();
  auto &&k = j["sentences"][0];
  auto sent = foo::classifier::make_sentence(k);
  auto c = foo::classifier::structure_cache_t{};

  auto i1 = foo::classifier::instance_t{sent, c, 2};
  REQUIRE(foo::classifier::is_legal(i1) == false);
  auto i2 = foo::classifier::instance_t{sent, c, 3};
  REQUIRE(foo::classifier::is_legal(i2) == false);
}
