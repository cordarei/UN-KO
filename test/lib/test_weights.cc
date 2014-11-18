#include <catch/catch.hpp>

#include <foo/weights.h>

#include <string>
#include <vector>


TEST_CASE("weights are initialized to zero and updated") {
  auto w = foo::weights_t{4};
  REQUIRE(0 == w.score({1,2,3,4}));
  w.update({1,2}, 1);
  REQUIRE(0 < w.score({1,2,3,4}));
  w.update({2,3}, -2);
  REQUIRE(0 > w.score({1,2,3,4}));
}

TEST_CASE("default constructed weights can be assigned to but not used") {
  foo::weights_t w{};
  w = foo::weights_t{1};
  REQUIRE(0 == w.score({1}));
}
