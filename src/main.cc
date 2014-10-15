#include <algorithm>
#include <iostream>
#include <iterator>

#include <json11/json11.hpp>
using json = json11::Json;



typedef std::string word_t;
typedef std::string symbol_t;
typedef std::string pos_t;
typedef size_t offset_t;

class span_t {
public:
  span_t(symbol_t l, offset_t b, offset_t e)
    : _label{l}, _begin{b}, _end{e}
  {}

  symbol_t const & label() const { return _label; }
  offset_t begin() const { return _begin; }
  offset_t end() const { return _end; }

private:
  symbol_t _label;
  offset_t _begin;
  offset_t _end;
};

std::ostream & operator<<(std::ostream &out, span_t const &span) {
  out << span.label() << ":[" << span.begin() << "," << span.end() << "]";
  return out;
}

class anchored_tree {
};

class parsed_sentence {
public:
  parsed_sentence() = delete;
  parsed_sentence(std::vector<word_t> words, std::vector<pos_t> tags, anchored_tree tree);

  anchored_tree const & tree();
  std::vector<word_t> const & words();
  std::vector<pos_t> const & tags();

private:
  std::vector<word_t> _words;
  std::vector<pos_t> _tags;
  anchored_tree _tree;
};


int main() {

  std::string allinput;
  allinput.reserve(1024);
  std::copy(
            std::istreambuf_iterator<char>(std::cin),
            std::istreambuf_iterator<char>(),
            std::back_inserter(allinput)
            );

  std::string err;
  json v = json::parse(allinput, err);

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
                  [](auto js) { return span_t(js[0].string_value(), js[1].int_value(), js[2].int_value()); });

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
