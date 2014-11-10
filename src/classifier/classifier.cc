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

    void register_features(feature_registry_t &/*reg*/, feature_config_t const &/*conf*/) {
    }


    /*
     * Commands
     */

    std::vector<sentence_t> read_sentences(std::istream &sin) {
      auto v = read_json(sin);
      return make_vector(v["sentences"].array_items(), make_sentence);
    }


    int train(docopt_t const &args) {
      //set up configuration from command-line arguments
      auto conf = make_config(args);

      //set up features
      auto reg = feature_registry_t{};
      register_features(reg, conf.features);

      //read in training file (JSON) and create instances
      std::istream * sin = &std::cin;
      std::ifstream fin{};
      if (conf.input_file != "-") fin.open(conf.input_file);

      // auto sentences = read_sentences(*sin);
      // auto caches = std::vector<structure_cache_t>{sentences.size()};
      auto sentences = make_vector(read_sentences(*sin), [](auto &&s) {
          return std::make_tuple(std::move(s), structure_cache_t{});
        });

      //generate features for each instance

      auto instances = make_vector(sentences, [&](auto const &swc) {
          auto & sent = std::get<0>(swc);
          auto & cache = std::get<1>(swc);
          auto len = sent.words.size();

          auto is = ranges::view::iota(offset_t{0})
            | ranges::view::take(len)
            | ranges::view::transform([&](auto &&sp) {
                auto i = instance_t{sent, cache, sp};
                auto fs = reg(i);
                ranges::sort(fs);
                return std::make_tuple(std::move(i), std::move(fs));
              });

          return make_vector(is);
        });

      // auto features = make_vector(instances, [&](auto const &is) {
      //     return make_vector(is, [](auto const &i) {
      //         auto fs = reg(i);
      //         ranges::sort(fs);
      //         return fs;
      //       });
      //   });

      //train weights using averaged perceptron

      //save weights and features

      return 0;
    }

    int test(docopt_t const &/*args*/) { return 0; }
    int run(docopt_t const &/*args*/) { return 0; }

  } //namespace classifier
} //namespace foo
