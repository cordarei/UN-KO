#include "parser.h"

#include <utility>
#include <memory>
#include <functional>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <regex>

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/view/to_container.hpp>
#include <range/v3/view/zip.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/range_for.hpp>

#include <foo/types.h>
#include <foo/span.h>
#include <foo/utility/container.h>
#include <range/v3/algorithm/sort.hpp>
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



    //
    struct rule_t {
      rule_t(std::string lhs, std::vector<std::string> rhs, double prob)
        : lhs_{std::move(lhs)}, rhs_{std::move(rhs)}, prob_{prob}
      {}
      rule_t();

      std::string const & lhs() const { return lhs_; }
      std::vector<std::string> const & rhs() const { return rhs_; }
      double const & prob() const { return prob_; }

    private:
      std::string lhs_;
      std::vector<std::string> rhs_;
      double prob_;
    };
    bool operator<(rule_t const &left, rule_t const &right) {
      return std::tie(left.lhs(), left.rhs()) < std::tie(right.lhs(), right.rhs());
    }
    bool operator>(rule_t const &left, rule_t const &right) {
      return std::tie(left.lhs(), left.rhs()) > std::tie(right.lhs(), right.rhs());
    }
    bool operator<=(rule_t const &left, rule_t const &right) {
      return !(left > right);
    }
    bool operator>=(rule_t const &left, rule_t const &right) {
      return !(left < right);
    }
    bool operator==(rule_t const &left, rule_t const &right) {
      return std::tie(left.lhs(), left.rhs()) == std::tie(right.lhs(), right.rhs());
    }
    bool operator!=(rule_t const &left, rule_t const &right) {
      return !(left == right);
    }

    //
    class grammar_t {
    public:
      void add_rule(rule_t rule) {
        rules_.emplace_back(std::move(rule));
        index_rule(rules_.back());
      }
      void sort() {
        lh_map_.clear();
        left_corner_map_.clear();

        ranges::sort(rules_);
        //ranges::sort(rules_, ranges::less{});
        //ranges::sort(rules_, std::less<>{});
        //std::sort(rules_.begin(), rules_.end());

        for (auto & r : rules_) {
          index_rule(r);
        }
      }

      auto find_lhs() const;

      auto find_left_corner(std::string const &label) const {
        auto empty = std::vector<rule_t const *>{};
        auto it = left_corner_map_.find(label);
        auto & rules = (it == left_corner_map_.end() ? empty : it->second);
        return ranges::view::transform(rules, [](rule_t const *r) { return *r; });
      }

    private:
      void index_rule(rule_t &r) {
        auto & lhv = lh_map_[r.lhs()];
        auto & lcv = left_corner_map_[r.rhs()[0]];
        lhv.push_back(&r);
        lcv.push_back(&r);
      }

      using map_t = std::unordered_map<std::string, std::vector<rule_t const *> >;

      std::vector<rule_t> rules_;
      map_t lh_map_;
      map_t left_corner_map_;
    };

    //
    class chart_t {
    public:
      struct item_t {
      public:
        using backpointers_t = std::vector<item_t const*>;

        item_t(rule_t const & r, backpointers_t bps, size_t b, size_t e, double w)
          : rule_{&r}, backpointers_{std::move(bps)}, begin_{b}, end_{e}, weight_{w}
        {}
        item_t(item_t const &) = default;
        item_t(item_t &&) = default;
        item_t & operator=(item_t const &) = default;
        item_t & operator=(item_t &&) = default;

        std::string const & label() const { return rule_->lhs(); }
        rule_t const & rule() const { return *rule_; }
        backpointers_t const & backpointers() const { return backpointers_; }
        size_t begin() const { return begin_; }
        size_t end() const { return end_; }
        double weight() const { return weight_; }

        bool complete() const { return backpointers_.size() == rule_->rhs().size(); }

      private:
        rule_t const * rule_;
        backpointers_t backpointers_;
        size_t begin_;
        size_t end_;
        double weight_;
      };

      using cell_t = std::vector<item_t>;

      explicit chart_t(size_t n) {
        chart_.resize(n);
        for (size_t i = 0; i < n; ++i) {
          chart_[i].resize(n - i);
        }
      }

      cell_t const & operator[](std::pair<size_t, size_t> p) const {
        return cell(p.first, p.second);
      }
      bool update(rule_t const & rule, item_t::backpointers_t backpointers, size_t begin, size_t end, double weight) {
        auto & cell = this->cell(begin, end);
        auto item = item_t{rule, std::move(backpointers), begin, end, weight};
        auto prev = ranges::find_if(cell, [&](auto && it) { return &item.rule() == &it.rule(); });
        if (prev == cell.end()) {
          cell.push_back(std::move(item));
          return true;
        }
        if (prev->weight() < weight) {
          *prev = std::move(item);
          return true;
        }

        return false;
      }

    private:
      cell_t const & cell(size_t i, size_t j) const {
        return chart_[i][j];
      }
      cell_t & cell(size_t i, size_t j) {
        return chart_[i][j];
      }

      using row_t = std::vector<cell_t>;
      std::vector<row_t> chart_;
    };

    //
    class tree_t {
    public:
      tree_t(std::string l) : label_{std::move(l)}, children_{} {}

      std::string const & label() const { return label_; }

      auto begin() { return children_.begin(); }
      auto end() { return children_.end(); }
      auto begin() const { return children_.begin(); }
      auto end() const { return children_.end(); }

      tree_t & add_child(std::string l) {
        children_.emplace_back(std::make_unique<tree_t>(std::move(l)));
        return *children_.back();
      }

    private:
      using child_t = std::unique_ptr<tree_t>;

      std::string label_;
      std::vector<child_t> children_;
    };

    std::ostream & operator<<(std::ostream & os, tree_t const &t) {
      os << "(" << t.label();
      for (auto && childptr : t) {
        os << " " << *childptr;
      }
      os << ")";
      return os;
    }

    //
    class parser_t {
    public:
      parser_t(grammar_t grammar) : grammar_{std::move(grammar)} {}

      tree_t parse(std::vector<std::string> const & words, std::vector<std::string> const & tags) const {
        size_t const n = tags.size();
        chart_t chart{n};

        auto lexical_rules =
          ranges::view::zip(tags,words)
          | ranges::view::transform([](auto && tup) { return rule_t{std::get<0>(tup), {std::get<1>(tup)}, 1.0}; })
          | ranges::view::to_vector;

        for (size_t i = 0; i < n; ++i) {
          //add tag[i] as complete span and all rules allowed by tag[i] as (in)complete spans to chart
          chart.update(lexical_rules[i], {}, i, i + 1, 1.0);
          introduce_items(chart, i, i + 1);
        }

        for (size_t j = 2; j <= n; ++j) {
          for (size_t i = 0; i <= (n - j); ++i) {
            for (size_t k = 1; k < j; ++k) {
              //add all (in)complete spans created by traversing the cross product of left x right
              auto & left = chart[{i, i + k}];
              auto & right = chart[{i + k, i + j}];

              for (auto const & left_item : left) {
                if (!left_item.complete()) {
                  for (auto const & right_item : right) {
                    if (right_item.complete()) {
                      chart.update(left_item.rule(),
                                   left_item.backpointers() + &right_item,
                                   i,
                                   i + j,
                                   left_item.weight() * right_item.weight());
                    }
                  }
                }
              }
              //then add (transitively) all (in)complete spans createable by the complete spans in chart[i,j]
              introduce_items(chart, i, j);
            }
          }
        }

        auto it = ranges::find_if(chart[{0, n}], [](auto && item) { return item.label() == "ROOT" || item.label() == "TOP"; });
        if (it == chart[{0, n}].end()) {
          return {"ERROR"};
        } else {
          return make_tree(*it);
        }
      }

      tree_t make_tree(chart_t::item_t const & item) const {
        tree_t tree = tree_t{item.label()};
        for (auto ptr : item.backpointers()) {
          make_tree_helper(tree, *ptr);
        }
        return tree;
      }

    private:
      void introduce_items(chart_t & chart, size_t i, size_t j) const {
        auto done = false;
        while (!done) {
          auto updated = false;
          for (auto && item : chart[{i,j}]) {
            if (item.complete()) {
              RANGES_FOR (auto && rule, grammar_.find_left_corner(item.label())) {
                updated = chart.update(rule, {&item}, i, j, rule.prob() * item.weight()) || updated;
              }
              if (updated) break;
            }
          }
          done = !updated;
        }
      }

      void make_tree_helper(tree_t & parent, chart_t::item_t const & item) const {
        auto & child = parent.add_child(item.label());
        for (auto ptr : item.backpointers()) {
          make_tree_helper(child, *ptr);
        }
      }

      grammar_t grammar_;
    };

    std::vector<std::string> split(std::string const &s, std::string const &pat) {
      auto result = std::vector<std::string>{};
      std::regex re(pat);
      std::copy(std::sregex_token_iterator(s.begin(), s.end(), re, -1),
                std::sregex_token_iterator(),
                std::back_inserter(result));
      return result;
    }

    grammar_t read_grammar(std::string const &filename) {
      std::ifstream fin{filename};
      auto grammar = grammar_t{};
      RANGES_FOR (auto && line, ranges::lines(fin)) {
        auto tabpos = line.find('\t');
        auto prob = std::stod(line.substr(tabpos + 1));
        auto s = line.substr(0, tabpos);
        auto ss = split(s, " ");
        auto lhs = ss.front();
        ss.erase(ss.begin());
        grammar.add_rule(rule_t{std::move(lhs), std::move(ss), prob});
      }
      grammar.sort();
      return grammar;
    }

    int run(docopt_t const & args) {
      auto conf = foo::classifier::make_config(args);
      auto parser = parser_t{read_grammar(conf.model_file)};
      std::ofstream fout{conf.output_file};

      foreach_sentence(conf.input_file, [&](auto const & js) mutable {
          auto words = make_vector(js["words"].array_items(), &json::string_value);
          auto tags  = make_vector(js["tags" ].array_items(), &json::string_value);
          auto tree = parser.parse(words, tags);
          fout << tree << std::endl;
        });

      return 0;
    }

  }
}
