#include <catch/catch.hpp>

#include <json11/json11.hpp>

constexpr auto JSON_STRING =
  u8R"({"sentences" : [
  {
    "words" : ["This", "is", "sentence", "one", "."],
    "tags" : ["DT", "VB", "NN", "NN", "."],
    "parse" : {
      "spans" : [["ROOT", 0, 5], ["S", 0, 5], ["NP", 0, 1], [null, 0, 1], ["VP", 1, 4], [null, 1, 2], ["NP", 2, 4], [null, 2, 3], [null, 3, 4], [null, 4, 5]],
      "edges" : [[0, [1]], [1, [2, 4, 9]], [2, [3]], [4, [5, 6]], [6, [7, 8]]]
    }
  },
  {
    "words" : ["This", "is", "sentence", "two", "."],
    "tags" : ["DT", "VB", "NN", "NN", "."],
    "parse" : {
      "spans" : [["ROOT", 0, 5], ["S", 0, 5], ["NP", 0, 1], [null, 0, 1], ["VP", 1, 4], [null, 1, 2], ["NP", 2, 4], [null, 2, 3], [null, 3, 4], [null, 4, 5]],
      "edges" : [[0, [1]], [1, [2, 4, 9]], [2, [3]], [4, [5, 6]], [6, [7, 8]]]
    }
  }
]
})";

TEST_CASE("we can parse the JSON") {
  std::string err;
  auto v = json11::Json::parse(JSON_STRING, err);
  REQUIRE(v["sentences"][0]["words"][3].string_value() == "one");
  REQUIRE(v["sentences"][1]["words"][3].string_value() == "two");
}
