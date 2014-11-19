#include "classifier.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <tuple>

#include <foo/features.h>
#include <foo/weights.h>
#include <foo/utility/container.h>
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


    using sent_cache_pair_t = std::tuple<sentence_t, structure_cache_t>;
    using sent_cache_vec_t = std::vector<sent_cache_pair_t>;

    sent_cache_vec_t make_sentences(std::string const & filename) {
      return make_vector(read_sentences(filename), [](auto &s) {
          return std::make_tuple(std::move(s), structure_cache_t{});
        });
    }

    template <typename FeatureFun>
    auto make_sentence_instance_range(sent_cache_vec_t const & sentences, FeatureFun && features) {
      return ranges::view::transform(sentences, [&features](auto const &swc) mutable {
            auto & sent = std::get<0>(swc);
            auto & cache = std::get<1>(swc);
            auto len = sent.words.size();
            return ranges::view::iota(offset_t{0})
                 | ranges::view::take(len)
                 | ranges::view::transform([&](auto &&sp) mutable {
                     auto i = instance_t{sent, cache, sp};
                     auto fs = features(i);
                     ranges::sort(fs);
                     return std::make_tuple(i, std::move(fs));
                   });
          });
    }


    template <typename Rng>
    weights_t train_binary(Rng & instances, size_t max_id) {
      auto w = weights_t{max_id};
      auto u = weights_t{max_id};
      auto c = 0;
      for (auto && tpl : instances) {
        auto & in = std::get<0>(tpl);
        auto & fs = std::get<1>(tpl);
        auto y_ = w.score(fs) > 0.;
        auto y = is_legal(in);
        if (y != y_) {
          auto z = (y ? 1 : -1);
          w.update(fs, z);
          u.update(fs, z * c);
        }
        ++c;
      }
      w.update(u, 1.0/c);
      return w;
    }

    template <typename Rng>
    weights_t train_multiclass(Rng & instances, update_t update, size_t max_id) {
      auto w = weights_t{max_id};
      auto u = weights_t{max_id};
      auto c = 0;
      for (auto && is : instances) {
        auto maxit = ranges::max_element(is, std::less<>(), [&w](auto && tpl) {
            auto & fs = std::get<1>(tpl);
            return w.score(fs);
          });
        auto y = is_legal(std::get<0>(*maxit));

        ++c;
      }
      w.update(u, 1.0/c);
      return w;
    }

    int train(docopt_t const & args) {
      //set up configuration from command-line arguments
      auto conf = make_config(args);

      //set up features
      auto features = make_feature_registry(conf.features);

      //read in training data
      auto sentences = make_sentences(conf.input_file);

      //create range over instances and features
      auto sent_inst_rng = make_sentence_instance_range(sentences, features);

      //train weights using averaged perceptron
      auto w = weights_t{};
      if (conf.update == update_t::binary) {
        auto instances = make_vector(sent_inst_rng | ranges::view::flatten);
        w = train_binary(instances, features.max_id());
      } else {
        auto instances = make_vector(sent_inst_rng, [](auto &&rng) { return make_vector(rng); });
        w = train_multiclass(instances, conf.update, features.max_id());
      }

      //save weights and features

      return 0;
    }


    int test(docopt_t const & args) {
      //set up configuration from command-line arguments
      auto conf = make_config(args);

      //set up features
      auto features = make_feature_registry(conf.features);
      {
        std::fstream sin{conf.feature_file};
        features.load(sin);
      }

      //read in training data
      auto sentences = make_sentences(conf.input_file);

      //create range over instances and features
      auto sent_inst_rng = make_sentence_instance_range(sentences, static_cast<feature_registry_t const&>(features));

      //read in weights
      auto w = weights_t{};//TODO

      if (conf.update == update_t::binary) {
        auto instances = make_vector(sent_inst_rng | ranges::view::flatten);
      } else {
        auto instances = make_vector(sent_inst_rng, [](auto &&rng) { return make_vector(rng); });
      }

      return 0;
    }

    int run(docopt_t const &/*args*/) { return 0; }

  } //namespace classifier
} //namespace foo
