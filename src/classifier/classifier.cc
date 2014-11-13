#include "classifier.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include <foo/utility/container.h>
#include <foo/features.h>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view/iota.hpp>

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
    void train_binary(T w, Rng & instances) {
      auto u = 0;//?
      auto c = 0;//?
      for (auto && tpl : instances) {
        auto & in = std::get<0>(tpl);
        auto & fs = std::get<1>(tpl);
      }
    }

    template <typename Rng>
    void train_multiclass(T w, Rng & instances) {
      auto u = 0;//?
      auto c = 0;//?
      for (auto && is : instances) {
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

      auto instances = make_vector(sentences, [&features](auto const &swc) mutable {
          auto & sent = std::get<0>(swc);
          auto & cache = std::get<1>(swc);
          auto len = sent.words.size();

          auto is = ranges::view::iota(offset_t{0})
            | ranges::view::take(len)
            | ranges::view::transform([&features](auto &&sp) mutable {
                auto i = instance_t{sent, cache, sp};
                auto fs = features(i);
                ranges::sort(fs);
                return std::make_tuple(std::move(i), std::move(fs));
              });

          return make_vector(is);
        });

      //train weights using averaged perceptron
      auto w = 0;//?
      if (conf.update == update_t::binary) {
        auto is = ranges::view::flatten(instances);
        train_binary(w, is);
      } else {
        //?
      }

      //save weights and features

      return 0;
    }

    int test(docopt_t const &/*args*/) { return 0; }
    int run(docopt_t const &/*args*/) { return 0; }

  } //namespace classifier
} //namespace foo
