#include <iostream>

#include <experimental/optional>

#include "picojson/picojson.h"

namespace json = picojson;

int main() {

  json::value v;
  std::cin >> v;
  if (std::cin.fail()) {
    std::cerr << json::get_last_error() << std::endl;
    return 1;
  }


  size_t sentence_count = 0;
  auto & sentences = v.get<json::object>()["sentences"].get<json::array>();
  for (auto & s : sentences) {
    ++sentence_count;
    std::cout << "sentence "
              << sentence_count
              << " size: "
              << s.get<json::object>()["words"].get<json::array>().size()
              << std::endl;
  }

  return 0;
}
