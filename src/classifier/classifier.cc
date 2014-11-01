#include "classifier.h"

#include <iostream>
#include <algorithm>

#include <range/v3/numeric.hpp>
#include <range/v3/lines_range.hpp>
#include <json11/json11.hpp>
#include <foo/foo.h>

using namespace foo;

  // struct lines_range : ranges::range_facade<lines_range> {
  // private:
  //   friend ranges::range_access;
  //   std::string _line;
  //   std::istream *_is;
  //   struct cursor {
  //   private:
  //     lines_range *_rng;
  //   public:
  //     cursor() = default;
  //     explicit cursor(lines_range &rng)
  //       : _rng(&rng)
  //     {}

  //     void next() {
  //       std::getline(*_rng->_is, _rng->_line);
  //     }
  //     std::string const &current() const {
  //       return _rng->_line;
  //     }
  //     bool done() const {
  //       return !*_rng->_is;
  //     }
  //   };
  //   cursor begin_cursor() { return cursor{*this}; }
  // public:
  //   lines_range() = default;
  //   lines_range(std::istream &is)
  //     : _is(&is)
  //   {
  //     std::getline(*_is, _line);
  //   }
  //   std::string &cached() { return _line; }
  // };

  // lines_range lines(std::istream &is) { return lines_range{is}; }

  std::string read_all(std::istream &is) {
    return ranges::accumulate(ranges::lines(is), std::string{});
    // return ranges::accumulate(lines(is), std::string{}, [](std::string a, std::string& b) { return a + b; });
  }

namespace {
  //↓temporary code
  json11::Json read_json(std::istream& is) {
    std::string err;
    return json11::Json::parse(read_all(is), err);
  }

  struct instance {
    std::vector<word_t> words;
    std::vector<pos_t> tags;
    std::vector<span_t> top_spans;
  };

  void create_instances() {
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
  }
}


int classifier::train() {
  //set up features
  //read in training file (JSON) and create instances
  //train weights using averaged perceptron
  //save weights and features
  return 0;
}


int classifier::test() { return 0; }
int classifier::run() { return 0; }
