#include <iostream>

#include <experimental/optional>

#include "picojson/picojson.h"


int main() {

  picojson::value v;
  std::cin >> v;
  if (std::cin.fail()) {
    std::cerr << picojson::get_last_error() << std::endl;
    return 1;
  }


  size_t sentence_count = 0;
  auto & sentences = v.get<picojson::object>()["sentences"].get<picojson::array>();
  for (auto & s : sentences) {
    ++sentence_count;
    std::cout << "sentence "
              << sentence_count
              << " size: "
              << s.get<picojson::object>()["words"].get<picojson::array>().size()
              << std::endl;
  }

  return 0;
}
