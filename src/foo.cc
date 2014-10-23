#include <algorithm>
#include <iostream>
#include <iterator>

#include <json11/json11.hpp>
json11::Json read_json(std::istream& is) {
  std::string err;
  std::string allinput;
  allinput.reserve(4096);

  std::copy(
            std::istreambuf_iterator<char>(is),
            std::istreambuf_iterator<char>(),
            std::back_inserter(allinput)
            );

  return json11::Json::parse(allinput, err);
}


#include <docopt/docopt.h>
constexpr auto USAGE =
  u8R"(foo

    Usage:
      foo [ARG...]

)";


#include <foo/foo.h>
using namespace foo;


int main(int argc, const char** argv) {

  auto args = docopt::docopt(USAGE,
                             { argv + 1, argv + argc },
                             true,
                             u8"foo -∞");

  auto v = read_json(std::cin);

  size_t sentence_count = 0;
  auto & sentences = v["sentences"].array_items();
  for (auto & s : sentences) {
    ++sentence_count;

    std::vector<word_t> words;
    words.reserve(s["words"].array_items().size());
    std::transform(begin(s["words"].array_items()),
                   end(s["words"].array_items()),
                   std::back_inserter(words),
                   [](auto js) { return js.string_value(); }
                   );

    std::vector<pos_t> tags;
    tags.reserve(s["tags"].array_items().size());
    std::transform(begin(s["tags"].array_items()),
                   end(s["tags"].array_items()),
                   std::back_inserter(tags),
                   [](auto js) { return js.string_value(); }
                   );

    std::vector<span_t> spans;
    auto const & jspans = s["parse"]["spans"].array_items();
    spans.reserve(jspans.size());
    std::transform(begin(jspans),
                   end(jspans),
                   std::back_inserter(spans),
                   [](auto js) { return span_t((js[0].is_string() ? js[0].string_value() : "<t>"), js[1].int_value(), js[2].int_value()); });

    std::cout << "sentence "
              << sentence_count
              << " num_words: "
              << words.size()
              << " num_tags: "
              << tags.size()
              << " num_spans: "
              << spans.size()
              << std::endl;

    for (auto const &edge : s["parse"]["edges"].array_items()) {
      auto const & head_span = spans[edge[0].int_value()];
      std::cout << "Edge: " << head_span << " =>";
      for (auto const &c : edge[1].array_items()) {
        std::cout << " " << spans[c.int_value()];
      }
      std::cout << std::endl;
    }

  }

  return 0;
}
