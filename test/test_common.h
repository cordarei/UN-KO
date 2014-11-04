#ifndef FOO_TEST_TEST_COMMON_H
#define FOO_TEST_TEST_COMMON_H


#include <json11/json11.hpp>


namespace {
  constexpr auto JSON_STRING = u8R"({"sentences" : [
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

  json11::Json get_json() {
    std::string err;
    return json11::Json::parse(JSON_STRING, err);
  }
}

#endif
