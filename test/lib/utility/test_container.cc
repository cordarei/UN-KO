#include <catch/catch.hpp>

#include <vector>
#include <string>
#include <type_traits>

#include <foo/utility/container.h>

TEST_CASE("we can make a vector from anything") {
  using namespace std::literals::string_literals;
  auto il = {"a"s, "ab"s, "abc"s};
  auto v1 = foo::make_vector(il);
  REQUIRE(v1 == (std::vector<std::string>{"a"s, "ab"s, "abc"s}));

  auto v2 = foo::make_vector(il, ranges::size);
  //auto v2 = foo::make_vector(il, [](auto && s){ return s.size(); });
  //auto v2 = foo::make_vector(il, &std::string::size);
  REQUIRE(v2 == (std::vector<size_t>{1, 2, 3}));

  auto v3 = foo::make_vector(std::string{"abc"});
  REQUIRE(v3 == (std::vector<char>{'a', 'b', 'c'}));

  auto v4 = foo::make_vector(std::string{"abc"}, [](auto &c) { return static_cast<int>(c); });
  REQUIRE(v4 == (std::vector<int>{97, 98, 99}));
}
