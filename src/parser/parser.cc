#include "parser.h"

#include <utility>
#include <chrono>
#include <memory>
#include <new>
#include <functional>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <regex>

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/algorithm/lower_bound.hpp>
#include <range/v3/algorithm/upper_bound.hpp>
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
    std::ostream & operator<<(std::ostream & os, rule_t const &r) {
      os << r.lhs() << " -> [";
      for (auto && s : r.rhs()) {
        os << " " << s;
      }
      os << " ](" << r.prob() << ")";
      return os;
    }

    class trie_t {
    private:
      static std::string const end_key;

      struct node_t {
        explicit node_t(rule_t const &r) : key_{trie_t::end_key}, data_{&r} {trace();}
        explicit node_t(std::string key) : key_{std::move(key)}, next_{} {trace();}
        ~node_t() {
          trace();
          if (key_ != trie_t::end_key) {
            next_.~vector<std::unique_ptr<node_t> >();
          }
        }
        node_t(node_t && n) : key_{std::move(n.key_)}, data_{} {
          trace();
          if (key_ != trie_t::end_key) {
            //next_ = std::move(n.next_);
            new (&next_) std::vector<std::unique_ptr<node_t> >(std::move(n.next_));
          } else {
            data_ = n.data_;
          }
        }

        std::string const & key() const { return key_; };

        node_t & add_child(rule_t const &r) {
          trace();
          assert(!end());
          next_.push_back(std::make_unique<node_t>(r));
          return *next_.back();
        }

        node_t & add_child(std::string const & k) {
          trace();
          assert(!end());
          for (auto & ptr : next_) {
            if (ptr->key_ == k)
              return *ptr;
          }
          next_.push_back(std::make_unique<node_t>(k));
          return *next_.back();
        }

        node_t const * get_child(std::string const & k) const {
          trace();
          if (key_ == trie_t::end_key)
            return nullptr;

          for (auto & ptr : next_) {
            if (ptr->key_ == k)
              return ptr.get();
          }

          return nullptr;
        }

        bool end() const { return key_ == trie_t::end_key; }
        rule_t const * get_end() const {
          if (end()) return data_;
          else return nullptr;
        }

      private:
        std::string const key_;
        union {
          std::vector<std::unique_ptr<node_t> > next_;
          rule_t const * data_;
        };
      };

      std::vector<node_t> roots_;

    public:
      class cursor {
        friend class trie_t;
        node_t const * cur_;
        cursor(node_t const * n) : cur_{n} {}
      public:
        cursor next(std::string const & key) const {
          assert(cur_ != nullptr);
          return cursor(cur_->get_child(key));
        }
        rule_t const * end() const {
          assert(cur_ != nullptr);
          auto n = cur_->get_child(trie_t::end_key);
          if (n) return n->get_end();
          else return nullptr;
        }
        explicit operator bool() const { return cur_ != nullptr; }
        bool operator==(cursor const &o) { return cur_ == o.cur_; }
        bool operator!=(cursor const &o) { return cur_ != o.cur_; }
      };

      void add_rule(rule_t const &rule) {
        trace();
        node_t * start = nullptr;
        for (auto & n : roots_) {
          if (rule.lhs() == n.key()) {
            start = &n;
          }
        }
        if (start == nullptr) {
          roots_.emplace_back(rule.lhs());
          start = &roots_.back();
        }
        for (auto && k : rule.rhs()) {
          start = &start->add_child(k);
        }
        start->add_child(rule);
      }

      cursor start(std::string const & key) const {
        for (auto & n : roots_) {
          if (n.key() == key)
            return cursor(&n);
        }
        return cursor(nullptr);
      }
      std::vector<node_t> const & roots() const { return roots_; }
    };
    std::string const trie_t::end_key = "<$>";

    //
    class grammar_t {
    public:
      void add_rule(rule_t rule) {
        trace();
        rules_.emplace_back(std::move(rule));
      }
      void index() {
        for (auto && rule : rules_) trie_.add_rule(rule);
      }

      trie_t const & trie() const { return trie_; }

    private:

      std::vector<rule_t> rules_;
      trie_t trie_;
    };

    //
    class chart_t {
    public:
      struct backpointer {
        size_t i;
        size_t j;
        size_t k;
        backpointer(size_t a, size_t b, size_t c) : i{a}, j{b}, k{c} {}
      };
      using backpointers_t = std::vector<backpointer>;
      using state_t = trie_t::cursor;

      struct item_t {
        item_t(backpointers_t bps, size_t b, size_t e, double w)
          : backpointers_{std::move(bps)}, begin_{b}, end_{e}, weight_{w}
        {}
        backpointers_t const & backpointers() const { return backpointers_; }
        size_t begin() const { return begin_; }
        size_t end() const { return end_; }
        double weight() const { return weight_; }

      private:
        backpointers_t backpointers_;
        size_t begin_;
        size_t end_;
        double weight_;
      };

      struct complete_item_t : item_t {
        complete_item_t(rule_t const & r, backpointers_t bps, size_t b, size_t e, double w)
          : item_t{std::move(bps), b, e, w}, rule_{&r}
        {}

        rule_t const & rule() const { return *rule_; }
        std::string const & label() const { return rule_->lhs(); }

      private:
        rule_t const * rule_;
      };

      struct incomplete_item_t : item_t {

        incomplete_item_t(state_t state, backpointers_t bps, size_t b, size_t e, double w)
          : item_t{std::move(bps), b, e, w}, state_{state}
        {}

        state_t state() const { return state_; }

      private:
        trie_t::cursor state_;
      };

      /*
      struct item_t {
      public:
        using backpointers_t = std::vector<item_t const*>;

        item_t(rule_t const & r, backpointers_t bps, size_t b, size_t e, double w)
          : rule_{&r}, backpointers_{std::move(bps)}, begin_{b}, end_{e}, weight_{w}
        {}
        item_t(item_t const &) = default;
        item_t(item_t &&o) : rule_{o.rule_}, backpointers_{std::move(o.backpointers_)}, begin_{o.begin_}, end_{o.end_}, weight_{o.weight_} {trace(); log(rule_);}
        item_t & operator=(item_t const &) = default;
        item_t & operator=(item_t &&o) {
          trace();
          rule_ = o.rule_;
          backpointers_ = std::move(o.backpointers_);
          begin_ = o.begin_;
          end_ = o.end_;
          weight_ = o.weight_;
          return *this;
        }

        std::string const & label() const { return rule_->lhs(); }
        rule_t const & rule() const { return *rule_; }
        backpointers_t const & backpointers() const { return backpointers_; }
        size_t begin() const { return begin_; }
        size_t end() const { return end_; }
        double weight() const { return weight_; }

        bool complete() const { return backpointers_.size() == rule_->rhs().size(); }
        std::string next() const {
          return (complete() ? "" : rule_->rhs()[backpointers_.size()]);
        }

        bool operator==(item_t const &other) {
          if (complete() != other.complete())
            return false;
          if (complete() && label() != other.label())
            return false;
          if (!complete() && (&rule() != &other.rule()))
            return false;
          if (backpointers().size() != other.backpointers().size())
            return false;
          if (begin() != other.begin() || end() != other.end())
            return false;

          return true;
        }
        bool operator!=(item_t const &other) { return !(*this == other); }

      private:
        rule_t const * rule_;
        backpointers_t backpointers_;
        size_t begin_;
        size_t end_;
        double weight_;
      };
      */

      class cell_t : std::pair<std::vector<complete_item_t>, std::vector<incomplete_item_t>> {
      public:
        using base_type = std::pair<std::vector<complete_item_t>, std::vector<incomplete_item_t>>;
        using base_type::base_type;
        cell_t() : base_type{{}, {}} {}

        std::vector<complete_item_t> const & complete() const { return this->first; }
        std::vector<incomplete_item_t> const & incomplete() const { return this->second; }
        std::vector<complete_item_t> & complete() { return this->first; }
        std::vector<incomplete_item_t> & incomplete() { return this->second; }
      };

      explicit chart_t(size_t n) {
        chart_.resize(n);
        for (size_t i = 0; i < n; ++i) {
          chart_[i].resize(n - i);
        }
      }

      cell_t const & operator[](std::pair<size_t, size_t> p) const {
        return cell(p.first, p.second);
      }
      complete_item_t const & operator[](backpointer bp) const {
        auto & c = cell(bp.i, bp.j).complete();
        assert(bp.k < c.size());
        return c[bp.k];
      }

      bool complete(rule_t const & rule, backpointers_t backpointers, size_t begin, size_t end, double weight) {
        // log("enter");
        auto & list = this->cell(begin, end).complete();
        auto prev = ranges::find_if(list, [&](auto const & item) { return item.rule().lhs() == rule.lhs(); });
        if (prev == list.end()) {
          // log("prev == list.end()");
          list.emplace_back(rule, std::move(backpointers), begin, end, weight);
          return true;
        }
        if (prev->weight() < weight) {
          // log("prev->weight() < weight");
          *prev = complete_item_t{rule, std::move(backpointers), begin, end, weight};
          return true;
        }
        // log("no update");
        return false;
      }

      bool update(state_t const &state, backpointers_t backpointers, size_t begin, size_t end, double weight) {
        // log("enter");
        auto & list = this->cell(begin, end).incomplete();
        auto prev = ranges::find_if(list, [&](auto const & item) { return item.state() == state; });
        if (prev == list.end()) {
          list.emplace_back(state, std::move(backpointers), begin, end, weight);
          return true;
        }
        if (prev->weight() < weight) {
          *prev = incomplete_item_t{state, std::move(backpointers), begin, end, weight};
          return true;
        }
        return false;
      }

      /*
      bool update(rule_t const & rule, item_t::backpointers_t backpointers, size_t begin, size_t end, double weight) {
        log("enter |" << rule << "| " << backpointers.size() << " " << begin << " " << end << " " << weight);
        auto item = item_t{rule, std::move(backpointers), begin, end, weight};
        auto complete = item.complete();
        auto & cell = this->cell(begin, end);
        auto & list = (complete ? cell.complete() : cell.incomplete());
        log("cell size: " << list.size());

        // auto prev = ranges::find_if(cell, [&](auto && it) { return match_item(it, item); });
        auto prev = ranges::find(list, item);
        if (prev == list.end()) {
          log("adding new item " << begin << " " << end << " " << weight << " complete? " << (item.complete() ? "yes" : "no") << " :: |" << rule << "|");
          list.push_back(std::move(item));
          log("new item address: " << &list.back());
          return complete;
        }
        if (prev->weight() < weight) {
          log("updating previous item "<< begin << " " << end << " " << weight << " prev weight: " << prev->weight() << " complete? " << (item.complete() ? "yes" : "no") << " :: |" << item.rule() << "| :: |" << prev->rule() << "|");
          *prev = std::move(item);
          return complete;
        }
        log("no update");

        return false;
      }
      */

    private:
      cell_t const & cell(size_t i, size_t j) const {
        return chart_[i][j - i - 1];
      }
      cell_t & cell(size_t i, size_t j) {
        return chart_[i][j - i - 1];
      }
      // bool match_item(item_t const &prev, item_t const &replace) {
      //   if (prev.complete() != replace.complete()) {
      //     return false;
      //   } else if (replace.complete()) {
      //     return prev.label() == replace.label();
      //   } else {
      //     return &prev.rule() == &replace.rule();
      //   }
      // }

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

      friend std::ostream & operator<<(std::ostream & os, tree_t const &t) {
        os << "(" << t.label();
        if (t.children_.size() == 1 && t.children_[0]->children_.size() == 0) {
          os << " " << t.children_[0]->label();
        } else {
          for (auto && childptr : t) {
            os << " " << *childptr;
          }
        }
        os << ")";
        return os;
      }

    private:
      using child_t = std::unique_ptr<tree_t>;

      std::string label_;
      std::vector<child_t> children_;
    };

    template <typename T>
    std::ostream &operator<<(std::ostream &os, std::vector<T> vec) {
      auto it = vec.begin();
      auto end = vec.end();
      os << "[";
      os << *it++;
      for (; it != end; ++it) {
        os << ", " << *it;
      }
      os << "]";
      return os;
    }

    //
    class parser_t {
    public:
      parser_t(grammar_t grammar) : grammar_{std::move(grammar)} {}

      tree_t parse(std::vector<std::string> const & words, std::vector<std::string> const & tags, std::vector<size_t> const & split_points) const {
        trace();
        size_t const n = tags.size();
        chart_t chart{n};
        log("initialized chart");
        log("split points: " << split_points);

        auto lexical_rules =
          tags
          | ranges::view::transform([](auto && t) { return rule_t{t, {}, 1.0}; })
          | ranges::view::to_vector;
        log("created lexical rules");

        for (size_t i = 0; i < n; ++i) {
          //std::cerr << "Filling cell (" << i << ", " << (i + 1) << ")" << std::endl;
          log("Filling cell (" << i << ", " << (i + 1) << ")");
          //add tag[i] as complete span and all rules allowed by tag[i] as (in)complete spans to chart
          chart.complete(lexical_rules[i], {}, i, i + 1, 1.0);
          introduce_items(chart, i, i + 1);
          log("Filled cell with " << (chart[{i, i + 1}].complete().size()) << " complete items and " <<  (chart[{i, i + 1}].incomplete().size()) << " incomplete items.");
          //std::cerr << "Filled cell with " << (chart[{i, i + 1}].complete().size()) << " complete items and " <<  (chart[{i, i + 1}].incomplete().size()) << " incomplete items." << std::endl;
        }
        log("finished initializing terminal cells");

        for (size_t j = 2; j <= n; ++j) {
          for (size_t i = 0; i <= (n - j); ++i) {
            //std::cerr << "Filling cell (" << i << ", " << (i + j) << ")" << std::endl;
            log("Filling cell (" << i << ", " << (i + j) << ")");
            auto cross = cross_independent_span(i, i + j, split_points);
            auto do_incomplete = !cross || i == 0;
            auto do_complete = !cross || (i == 0 && j == n);
            log("Constraints: cross:" << cross << " do_incomplete:" << do_incomplete << " do_complete:" << do_complete);
            if (!do_incomplete)
              continue;
            for (size_t k = 1; k < j; ++k) {
              //add all (in)complete spans created by traversing the cross product of left x right
              auto & left = chart[{i, i + k}].incomplete();
              auto & right = chart[{i + k, i + j}].complete();
              auto bp = chart_t::backpointer{i + k, i + j, 0};

              for (auto const & left_item : left) {
                bp.k = 0;
                for (auto const & right_item : right) {
                  auto state = left_item.state().next(right_item.rule().lhs());
                  if (state) {
                    chart.update(state,
                                 left_item.backpointers() + bp,
                                 i,
                                 i + j,
                                 left_item.weight() * right_item.weight());
                    auto rule = state.end();
                    if (rule != nullptr && do_complete) {
                      chart.complete(*rule,
                                     left_item.backpointers() + bp,
                                     i,
                                     i + j,
                                     left_item.weight() * right_item.weight() * rule->prob());
                    }
                  }
                  ++bp.k;
                }
              }
            }
            //then add (transitively) all (in)complete spans createable by the complete spans in chart[i,j]
            if (do_complete)
              introduce_items(chart, i, i + j);
            log("Filled cell with " << (chart[{i, i + j}].complete().size()) << " complete items and " <<  (chart[{i, i + j}].incomplete().size()) << " incomplete items.");
            //std::cerr << "Filled cell with " << (chart[{i, i + j}].complete().size()) << " complete items and " <<  (chart[{i, i + j}].incomplete().size()) << " incomplete items." << std::endl;
          }
        }
        log("finished filling chart");
        // std::cerr << "Finished parsing. Top complete items: ";
        // for (auto && item : chart[{0, n}].complete()) {
        //   std::cerr << " " << item.label() << "=" << item.weight();
        // }
        // std::cerr << std::endl;

        auto it = ranges::find_if(chart[{0, n}].complete(), [](auto && item) { return item.label() == "ROOT" || item.label() == "TOP"; });
        if (it == chart[{0, n}].complete().end()) {
          std::cerr << "No complete parse" << std::endl;
          return {"ERROR"};
        } else {
          std::cerr << "Parse with " << it->rule().rhs()[0] << std::endl;
          return make_tree(chart, words, *it);
        }
      }

    private:
      bool cross_independent_span(size_t i, size_t j, std::vector<size_t> const & split_points) const {
        log(i << " " << j << " " << split_points);
        for (auto sp : split_points)
          if (i < sp && j > sp)
            return true;
        return false;
      }

      void introduce_items(chart_t & chart, size_t i, size_t j) const {
        trace();
          // for (auto const & item : chart[{i,j}].complete()) {
          //   log("check complete item address " << &item << " " << &item.rule());
          // }
        auto done = false;
        while (!done) {
          // log("enter while loop");
          auto updated = false;
          auto bp = chart_t::backpointer{i, j, 0};
          for (auto const & item : chart[{i,j}].complete()) {
            // log("complete item " << &item << " " << &item.rule());
            auto const & category = item.label();
            // log("introducing rules for complete edge: " << category);
            for (auto const & trienode : grammar_.trie().roots()) {
              auto state = grammar_.trie().start(trienode.key()).next(category);
              if (state) {
                // log("valid state");
                auto rule = state.end();
                chart.update(state, {bp}, i, j, item.weight());
                if (rule != nullptr) {
                  // log("complete state");
                  bool upd = chart.complete(*rule, {bp}, i, j, item.weight() * rule->prob());
                  updated = upd || updated;
                }
              }
            }
            // RANGES_FOR (rule_t const* rule, grammar_.find_left_corner(category)) {
            //   log("updating with rule " << rule);
            //   ++count;
            //   updated = chart.update(*rule, {&item}, i, j, rule->prob() * item.weight()) || updated;
            //   log("updated? " << (updated ? "yes" : "no"));
            //   if (updated) break;
            // }
            // log("explored " << count << " rules.");
            if (updated) break;
            ++bp.k;
          }
          done = !updated;
        }
      }

      tree_t make_tree(chart_t const & chart, std::vector<std::string> const & words, chart_t::complete_item_t const & item) const {
        // std::cerr << "make_tree() enter" << std::endl;
        // std::cerr << "<root rule=|" << item.rule() << "|>" << std::endl;
        tree_t tree = tree_t{item.label()};
        for (auto bp : item.backpointers()) {
          // auto & ch = chart[bp];
          // std::cerr << "  (" << bp.i << "," << bp.j << "," << bp.k << ") : " << ch.rule() << std::endl;
          // std::cerr << "<item rule=|" << item.rule() << "| bp=|"<< bp.i << "," << bp.j << "," << bp.k << "|>" << std::endl;
          make_tree_helper(chart, words, tree, chart[bp]);
          // std::cerr << "</item>" << std::endl;
        }
        if (item.backpointers().empty()) {
          tree.add_child(words[item.begin()]);
        }
        // std::cerr << "</root>" << std::endl;
        return tree;
      }

      void make_tree_helper(chart_t const & chart, std::vector<std::string> const & words, tree_t & parent, chart_t::complete_item_t const & item) const {
        auto & child = parent.add_child(item.label());
        for (auto bp : item.backpointers()) {
          // std::cerr << "<item rule=|" << item.rule() << "| bp=|"<< bp.i << "," << bp.j << "," << bp.k << "| w=|" << item.weight() << "|>" << std::endl;
          make_tree_helper(chart, words, child, chart[bp]);
          // std::cerr << "</item>" << std::endl;
        }
        if (item.backpointers().empty()) {
          child.add_child(words[item.begin()]);
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
      trace();
      std::ifstream fin{filename};
      auto grammar = grammar_t{};
      RANGES_FOR (auto && line, ranges::lines(fin)) {
        auto tabpos = line.find('\t');
        auto prob = std::stod(line.substr(tabpos + 1));
        auto s = line.substr(0, tabpos);
        auto ss = split(s, " ");
        auto lhs = ss.front();
        ss.erase(ss.begin());
        if (lhs != ss.front() || ss.size() > 1)
          grammar.add_rule(rule_t{lhs, ss, prob});
      }
      // grammar.sort();
      grammar.index();
      return grammar;
    }

    class timer
    {
    private:
      std::chrono::high_resolution_clock::time_point start_;
    public:
      timer()
      {
        reset();
      }
      void reset()
      {
        start_ = std::chrono::high_resolution_clock::now();
      }
      std::chrono::milliseconds elapsed() const
      {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_);
      }
      friend std::ostream &operator<<(std::ostream &sout, timer const &t)
      {
        return sout << t.elapsed().count() << "ms";
      }
    };

    int run(docopt_t const & args) {
      trace();
      auto conf = foo::classifier::make_config(args);
      auto parser = parser_t{read_grammar(conf.model_file)};
      std::ofstream fout{conf.output_file};

      auto count = size_t{0};
      foreach_sentence(conf.input_file, [&](auto const & js) mutable {
          std::cerr << "Parsing sentence " << count++ << "..." << std::endl;
          auto words = make_vector(js["words"].array_items(), &json::string_value);
          auto tags  = make_vector(js["tags" ].array_items(), &json::string_value);
          auto split_points = std::vector<size_t>{};
          if (!js["split_points"].is_null()) {
            split_points = make_vector(js["split_points"].array_items(), [](auto && sp) { return static_cast<size_t>(sp.int_value()); });
            ranges::sort(split_points);
          }
          if (conf.parse_oracle) {
            split_points = js["parse"]["edges"][1][1].array_items()
              | ranges::view::transform(&json::int_value)
              | ranges::view::transform([&](auto i) {
                  return static_cast<size_t>(js["parse"]["spans"][i][2].int_value());
                })
              | ranges::view::to_vector;
            split_points.erase(--split_points.end());
            log("Using oracle; constraints: " << split_points);
          }
          timer t;
          auto tree = parser.parse(words, tags, split_points);
          if (tree.label() == "ERROR" && !split_points.empty()) {
            std::cout << "Re-parsing without constraints." << std::endl;
            tree = parser.parse(words, tags, {});
          }
          auto elapsed = t.elapsed().count();
          std::cerr << "Parsed " << words.size() << " words in " << elapsed << "ms." << std::endl;
          fout << tree << std::endl;
        });

      return 0;
    }

  }
}
