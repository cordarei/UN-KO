#include "parser.h"

#include <utility>
#include <fstream>
#include <iostream>

#include <range/v3/algorithm/for_each.hpp>

#include <foo/types.h>
#include <foo/span.h>
#include <foo/utility/container.h>
#include "../classifier/config.h"


namespace foo {
  namespace parser {


    template <typename Fun>
    void foreach_sentence(std::string const &filename, Fun && fun) {
      std::ifstream fin{filename};
      auto v = read_json(fin);
      ranges::for_each(v["sentences"].array_items(), std::forward<Fun>(fun));
    }

    int extract_grammar(docopt_t const & args) {
      using rule_t = std::pair<std::string, std::string>;

      auto conf = foo::classifier::make_config(args);

      auto total_count = size_t{0};
      auto lh_counts = std::map<std::string, size_t>{};
      auto rule_counts = std::map<rule_t, size_t>{};

      foreach_sentence(conf.input_file, [&](auto const & js) mutable {
          auto words = make_vector(js["words"].array_items(), &json::string_value);
          auto tags  = make_vector(js["tags" ].array_items(), &json::string_value);
          auto spans = make_vector(js["parse"]["spans"].array_items(), [&tags](auto const &span) {
              auto b = static_cast<offset_t>(span[1].int_value());
              auto e = static_cast<offset_t>(span[2].int_value());
              auto l = (span[0].is_string() ? span[0].string_value() : tags[b]);
              return span_t{l, b, e};
            });
          ranges::for_each(js["parse"]["edges"].array_items(), [&](auto const &edge) mutable {
              auto lh = spans[edge[0].int_value()].label();
              auto rh = std::string{};
              ranges::for_each(edge[1].array_items(), [&](auto const &idx) mutable {
                  rh += spans[idx.int_value()].label();
                  rh += " ";
                });
              rh.erase(rh.size() - 1);
              ++lh_counts[lh];
              ++rule_counts[make_pair(lh, rh)];
              ++total_count;
            });
        });

      std::ofstream fout{conf.output_file};

      for (auto const & pr : rule_counts) {
        auto n = static_cast<double>(pr.second);
        auto d = static_cast<double>(lh_counts[pr.first.first]);
        auto prob = n / d;
        fout << pr.first.first << " " << pr.first.second << "\t" << prob << std::endl;
      }

      std::cerr << "LH Counts: " << lh_counts.size() << std::endl;
      std::cerr << "Rule Counts: " << rule_counts.size() << std::endl;
      std::cerr << "Production Counts: " << total_count << std::endl;

      return 0;
    }

    int run(docopt_t const & args) {
      return 0;
    }

  }
}
