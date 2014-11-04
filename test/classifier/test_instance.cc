#include <catch/catch.hpp>

#include "../test_common.h"
#include "../../src/classifier/instance.h"


TEST_CASE("instance can be default and copy constructed") {
  using foo::classifier::instance;
  instance i{};
  auto i2 = instance{};
  auto i3{i2};
  auto i4 = i3;
}

TEST_CASE("instance can be created from a JSON object") {
  auto j = get_json();
  auto &&k = j["sentences"][0];
  auto i = foo::classifier::make_instance(k);

  REQUIRE(i.words == (std::vector<std::string>{"This", "is", "sentence", "one", "."}));
  REQUIRE(i.top_spans == (std::vector<foo::span_t>{
        {"NP", 0u, 1u}, {"VP", 1u, 4u}, {".", 4u, 5u}
      }));
}
