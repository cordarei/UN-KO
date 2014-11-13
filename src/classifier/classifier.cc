#include "classifier.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include <foo/utility/container.h>
#include <foo/features.h>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/algorithm/transform.hpp>
#include <range/v3/algorithm/max_element.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/flatten.hpp>

#include "instance.h"
#include "config.h"


namespace foo {
  namespace classifier {

    /*
     * Features
     */

    using feature_registry_t = foo::feature_registry_t<foo::classifier::instance_t, std::string>;

    feature_registry_t make_feature_registry(feature_config_t const &/*conf*/) {
      return {};
    }


    /*
     * Commands
     */

    std::vector<sentence_t> read_sentences(std::istream &sin) {
      auto v = read_json(sin);
      return make_vector(v["sentences"].array_items(), make_sentence);
    }

    std::vector<sentence_t> read_sentences(std::string const &filename) {
      std::istream * sin = &std::cin;
      std::ifstream fin{};
      if (filename != "-") {
        fin.open(filename);
        sin = &fin;
      }

      return read_sentences(*sin);
    }


    template <typename Rng>
    void train_binary(Rng & instances) {
      auto w = 0;
      auto u = w;//?
      auto c = 0;//?
      for (auto && tpl : instances) {
        auto & in = std::get<0>(tpl);
        auto & fs = std::get<1>(tpl);
        auto y_ = (w * fs) > 0.;
        auto y = is_legal(in);
        if (y != y_) {
          auto z = (y ? 1 : -1);
          w += z * fs;
          u += z * c * fs;
        }
        ++c;
      }
      w -= (1./c) * u;
    }

    template <typename Rng>
    void train_multiclass(Rng & instances, update_t update) {
      auto w = 0;
      auto u = w;//?
      auto c = 0;//?
      for (auto && is : instances) {
        auto maxit = ranges::max_element(scores, std::less<>(), [&w](auto && tpl) {
            auto & fs = std::get<1>(tpl);
            return w * fs;
          });
        auto y = is_legal(std::get<0>(*maxit));
      }
    }


    int train(docopt_t const &args) {
      //set up configuration from command-line arguments
      auto conf = make_config(args);

      //set up features
      auto features = make_feature_registry(conf.features);

      auto sentences = make_vector(read_sentences(conf.input_file), [](auto &s) {
          return std::make_tuple(std::move(s), structure_cache_t{});
        });

      auto sent_inst_rng = sentences
        | ranges::view::transform([&ffun=features](auto const &swc) mutable {
            auto & sent = std::get<0>(swc);
            auto & cache = std::get<1>(swc);
            auto len = sent.words.size();
            return ranges::view::iota(offset_t{0})
                 | ranges::view::take(len)
                 | ranges::view::transform([&](auto &&sp) mutable {
                     auto i = instance_t{sent, cache, sp};
                     auto fs = features(i);
                     ranges::sort(fs);
                     return std::make_tuple(std::move(i), std::move(fs));
                   });
          });

      //train weights using averaged perceptron
      auto w = 0;//?
      if (conf.update == update_t::binary) {
        auto instances = make_vector(sent_inst_rng | ranges::view::flatten());
        w = train_binary(instances);
      } else {
        auto instances = make_vector(sent_inst_rng, [](auto &&rng) { return make_vector(rng); });
        w = train_multiclass(instances, conf.update);
      }

      //save weights and features

      return 0;
    }

    int test(docopt_t const &/*args*/) { return 0; }
    int run(docopt_t const &/*args*/) { return 0; }

  } //namespace classifier
} //namespace foo
