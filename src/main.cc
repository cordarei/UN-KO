#include <algorithm>
#include <iostream>
#include <iterator>

#include <json11/json11.hpp>
using json = json11::Json;



typedef std::string word_type;
typedef std::string pos_type;

class anchored_tree {
};

class parsed_sentence {
public:
  anchored_tree tree();
  std::vector<word_type> words();
  std::vector<pos_type> tags();
};


int main() {

  std::string err;
  std::string allinput;
  allinput.reserve(1024);
  std::copy(
            std::istreambuf_iterator<char>(std::cin),
            std::istreambuf_iterator<char>(),
            std::back_inserter(allinput)
            );

  json v = json::parse(allinput, err);

  size_t sentence_count = 0;
  auto & sentences = v["sentences"].array_items();
  for (auto & s : sentences) {
    ++sentence_count;
    std::cout << "sentence "
              << sentence_count
              << " size: "
              << s["words"].array_items().size()
              << std::endl;
  }

  return 0;
}
