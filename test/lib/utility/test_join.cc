#include <catch/catch.hpp>

#include <foo/utility/join.h>


TEST_CASE("join concatenates strings when called with no delimiter") {
  auto v = std::vector<std::string>{"a", "b", "c"};
  auto s = foo::join(v);
  REQUIRE(s == "abc");
}

TEST_CASE("join inserts delimiter between strings") {
  auto v = std::vector<std::string>{"a", "b", "c"};
  auto s = foo::join(v, ",");
  REQUIRE(s == "a,b,c");
}

struct S {
  std::string str;
};
TEST_CASE("join uses projection") {
  auto v = std::vector<S>{{"a"}, {"b"}, {"c"}};
  auto s = foo::join(v, "", &S::str);
  REQUIRE(s == "abc");
}
