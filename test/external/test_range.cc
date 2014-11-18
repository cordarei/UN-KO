#include <catch/catch.hpp>

#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/range_for.hpp>


TEST_CASE("accumulate with strings") {
  auto v = std::vector<std::string>{"foo", "bar", "baz"};

  REQUIRE(ranges::accumulate(v, std::string{}) == "foobarbaz");
  //REQUIRE(ranges::accumulate(v, std::string{}, [](auto &a, auto const &b) { return a += b; }) == "foobarbaz");
}

// TEST_CASE("transform with mutable lambda") {
//   auto v = std::vector<std::string>{"a", "ab", "abc"};
//   auto r = ranges::view::transform(v, [](auto const &s) { return s.size(); });
//   RANGES_FOR(auto const& e, r) {
//     INFO(e);
//   }
//   auto r2 = ranges::view::filter(v, [](auto const &s) { return s.size() > 1; });
//   RANGES_FOR(auto const& e, r2) {
//     INFO(e);
//   }
//   // std::vector<size_t> v2 = r;
//   // REQUIRE(v2 == (std::vector<size_t>{1, 2, 3}));
// }
