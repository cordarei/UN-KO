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
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/flatten.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/slice.hpp>
#include <range/v3/range_for.hpp>
#include <range/v3/action/push_back.hpp>

#include "instance.h"
#include "config.h"
#include "results.h"

using namespace std::string_literals;

namespace foo {
  namespace classifier {

    size_t subclamp(size_t left, size_t right) {
      if (left < right) {
        return 0;
      } else {
        return left - right;
      }
    }

    template <typename S>
    std::string concat(S && s) {
      auto result = std::string{std::forward<S>(s)};
      return result;
    }

    template <typename S1, typename S2, typename ...Ss>
    std::string concat(S1 && s1, S2 && s2, Ss &&... rest) {
      auto result = std::string{std::forward<S1>(s1)};
      result += std::forward<S2>(s2);
      if (0 < sizeof...(Ss)) {
        result = concat(std::move(result), std::forward<Ss>(rest)...);
      }
      return result;
    }

    /*
     * Features
     */

    using feature_registry_t = foo::feature_registry_t<foo::classifier::instance_t, std::string>;

    std::vector<std::string> global_pos_unigram_features(instance_t const & instance) {
      log("enter");
      auto left = instance.sentence().tags | ranges::view::take(instance.sp());
      auto right = instance.sentence().tags | ranges::view::drop(instance.sp());

      auto fvs = std::vector<std::string>{};

      ranges::push_back(fvs, left | ranges::view::transform([](auto & ug) {
            return concat("left_unigram:", ug);
          }));
      ranges::push_back(fvs, right | ranges::view::transform([](auto & ug) {
            return concat("right_unigram:", ug);
          }));

      log("leave");
      return fvs;
    }

    std::vector<std::string> global_pos_bigram_features(instance_t const & instance) {
      if (instance.cache().pos_bigrams.empty()) {
        instance.cache().pos_bigrams = bigrams(instance.sentence().tags);
      }

      auto left = instance.cache().pos_bigrams | ranges::view::take(subclamp(instance.sp(), 1u));
      auto right = instance.cache().pos_bigrams | ranges::view::drop(instance.sp());

      auto fvs = std::vector<std::string>{};

      ranges::push_back(fvs, left | ranges::view::transform([](auto & bg) {
            return concat("left_bigram:", std::get<0>(bg), "^", std::get<1>(bg));
          }));
      ranges::push_back(fvs, right | ranges::view::transform([](auto & bg) {
            return concat("right_bigram:", std::get<0>(bg), "^", std::get<1>(bg));
          }));

      return fvs;
    }

    std::vector<std::string> global_pos_trigram_features(instance_t const & instance) {
      log("enter");
      if (instance.cache().pos_trigrams.empty()) {
        instance.cache().pos_trigrams = trigrams(instance.sentence().tags);
      }

      auto left = instance.cache().pos_trigrams | ranges::view::take(subclamp(instance.sp(), 2u));
      auto right = instance.cache().pos_trigrams | ranges::view::drop(std::min(instance.cache().pos_trigrams.size(), instance.sp()));
      log(instance.sentence().tags.size() << " " << instance.cache().pos_trigrams.size() << " " << instance.sp() << " " << ranges::distance(right));

      auto fvs = std::vector<std::string>{};

      log("do left");
      ranges::push_back(fvs, left | ranges::view::transform([](auto & tg) {
            return concat("left_trigram:", std::get<0>(tg), "^", std::get<1>(tg), "^", std::get<2>(tg));
          }));
      log("do right");
      ranges::push_back(fvs, right | ranges::view::transform([](auto & tg) {
            return concat("right_trigram:", std::get<0>(tg), "^", std::get<1>(tg), "^", std::get<2>(tg));
          }));

      log("leave");
      return fvs;
    }

    std::vector<std::string> global_pos_prefix_features(instance_t const & instance) {
      auto fvs = std::vector<std::string>{};

      auto & tags = instance.sentence().tags;
      auto len = tags.size();
      auto sp = instance.sp();

      for (size_t i : {1, 2, 3, 4}) {
        if (sp >= i) {
          fvs.push_back(concat("global_pos_prefix_left:", foo::join(tags | ranges::view::take(i), "^")));
          fvs.push_back(concat("global_pos_suffix_left:", foo::join(tags | ranges::view::slice(sp - i, sp), "^")));
        }
        if (len >= (sp + i)) {
          fvs.push_back(concat("global_pos_prefix_right:", foo::join(tags | ranges::view::slice(sp, sp + i), "^")));
          fvs.push_back(concat("global_pos_suffix_right:", foo::join(tags | ranges::view::slice(len - i, len), "^")));
        }
      }
      return fvs;
    }

    std::vector<std::string> local_pos_features(instance_t const & instance) {
      auto fvs = std::vector<std::string>{};

      auto & tags = instance.sentence().tags;
      auto len = tags.size();
      auto sp = instance.sp();

      for (auto i = subclamp(sp, 3); i < sp; ++i) {
        auto left = tags | ranges::view::slice(i, sp);
        fvs.push_back(concat("local_pos_left:", foo::join(left, "^")));
      }
      for (auto i = std::min(sp, sp + 3); i > sp; --i) {
        auto right = tags | ranges::view::slice(sp, i);
        fvs.push_back(concat("local_pos_right:", foo::join(right, "^")));
      }
      for (size_t i : {1, 2, 3}) {
        if (sp >= i && len >= (sp + i)) {
          auto left = tags | ranges::view::slice(subclamp(sp, i), sp);
          auto right = tags | ranges::view::slice(sp, sp + i);
          fvs.push_back(concat("local_pos_around:", foo::join(left, "^"), "|", foo::join(right, "^")));
        }
      }

      return fvs;
    }

    feature_registry_t make_feature_registry(feature_config_t const & conf) {
      auto features = feature_registry_t{};
      if (conf.global) {
        if (conf.pos) {
          features.add_feature(global_pos_unigram_features);
          features.add_feature(global_pos_bigram_features);
          features.add_feature(global_pos_trigram_features);
        }
      }
      if (conf.local) {
        if (conf.pos) {
          features.add_feature(local_pos_features);
        }
      }
      return features;
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
    auto make_sentence_instance_range(sent_cache_vec_t & sentences, FeatureFun && features) {
      return ranges::view::transform(sentences, [&features](auto & swc) mutable {
            auto & sent = std::get<0>(swc);
            auto & cache = std::get<1>(swc);
            auto len = sent.words.size();
            return ranges::view::iota(offset_t{1})
                 | ranges::view::take(len - 1)
                 | ranges::view::transform([&](auto sp) mutable {
                     auto i = instance_t{sent, cache, sp};
                     auto fs = features(i);
                     ranges::sort(fs);
                     return std::make_tuple(i, std::move(fs));
                   });
          });
    }


    template <typename Rng>
    weights_t train_binary(Rng && instances, size_t max_id) {
      auto w = weights_t{max_id};
      auto u = weights_t{max_id};
      auto c = 0;
      // for (auto && tpl : instances) {
      RANGES_FOR(auto && tpl, instances) {
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
      w.update(u, -1.0/c);
      return w;
    }

    // template <typename Rng>
    // weights_t train_multiclass(Rng & instances, update_t update, size_t max_id) {
    //   auto w = weights_t{max_id};
    //   auto u = weights_t{max_id};
    //   auto c = 0;
    //   for (auto && is : instances) {
    //     auto maxit = ranges::max_element(is, std::less<>(), [&w](auto && tpl) {
    //         auto & fs = std::get<1>(tpl);
    //         return w.score(fs);
    //       });
    //     auto y = is_legal(std::get<0>(*maxit));

    //     ++c;
    //   }
    //   w.update(u, 1.0/c);
    //   return w;
    // }

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
      if (conf.classifier == classifier_type_t::binary) {
        auto instances = sent_inst_rng | ranges::view::flatten;
        ranges::for_each(instances, [](auto && x) {});
        w = train_binary(instances, features.max_id());
      } else {
        // auto instances = make_vector(sent_inst_rng, [](auto &&rng) { return make_vector(rng); });
        // w = train_multiclass(instances, conf.update, features.max_id());
      }

      //save weights and features
      {
        std::ofstream fout{conf.feature_file};
        features.save(fout);
      }
      {
        std::ofstream fout{conf.weights_file};
        w.save(fout);
      }

      return 0;
    }


    int test(docopt_t const & args) {
      //set up configuration from command-line arguments
      auto conf = make_config(args);

      //set up features
      auto features = make_feature_registry(conf.features);
      {
        std::ifstream sin{conf.feature_file};
        features.load(sin);
      }

      //read in training data
      auto sentences = make_sentences(conf.input_file);

      //create range over instances and features
      auto sent_inst_rng = make_sentence_instance_range(sentences, static_cast<feature_registry_t const&>(features));

      //read in weights
      auto w = weights_t{features.max_id()};
      {
        std::ifstream fin{conf.weights_file};
        w.load(fin);
      }

      if (conf.classifier == classifier_type_t::binary) {
        auto instances = sent_inst_rng | ranges::view::flatten;
        auto results = classification_results{};
        ranges::for_each(instances, [&results,&w](auto && tpl) {
            using result_t = classification_results::result_t;
            auto & in = std::get<0>(tpl);
            auto & fs = std::get<1>(tpl);
            auto y_ = w.score(fs) > 0.;
            auto y = is_legal(in);
            auto res = (y ? (y_ ? result_t::tp : result_t::fn) : (y_ ? result_t::fp : result_t::tn));
            results.add(res);
          });

        std::cout << "Binary Classifier Results:" << std::endl;
        std::cout << "TP\tFP\tTN\tFN" << std::endl;
        std::cout << results.tp() << "\t"
                  << results.fp() << "\t"
                  << results.tn() << "\t"
                  << results.fn() << "\t" << std::endl;;
        std::cout << "Accuracy: \t" << results.accuracy() << std::endl;
        std::cout << "Recall: \t" << results.recall() << std::endl;
        std::cout << "Precision: \t" << results.precision() << std::endl;
        std::cout << "F1: \t" << results.f1() << std::endl;

      } else {
        // auto instances = make_vector(sent_inst_rng, [](auto &&rng) { return make_vector(rng); });
      }

      return 0;
    }

  } //namespace classifier
} //namespace foo
