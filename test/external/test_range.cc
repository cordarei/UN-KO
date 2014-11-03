#include <catch/catch.hpp>

#include <range/v3/numeric/accumulate.hpp>


TEST_CASE("accumulate with strings") {
  auto v = std::vector<std::string>{"foo", "bar", "baz"};

  REQUIRE(ranges::accumulate(v, std::string{}) == "foobarbaz");
  //REQUIRE(ranges::accumulate(v, std::string{}, [](auto &a, auto const &b) { return a += b; }) == "foobarbaz");
}
