#include "classifier.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include <foo/utility/container.h>
#include <foo/features.h>
#include <range/v3/algorithm/sort.hpp>

#include "instance.h"


namespace foo {
  namespace classifier {

    /*
     * Config
     */

    using feature_register_t = foo::feature_registry_t<foo::classifier::instance, std::string>;

    struct feature_config_t {
      bool pos = false;
      bool word = false;
      bool global = false;
      bool local = false;
    };

    struct config_t {
      feature_config_t features;
      std::string input_file;
      std::string output_file; //optional
      std::string model_file;
      std::string feature_file;
    };

    config_t make_config(docopt_t const &args) {
      return {};
    }


    /*
     * Features
     */

    void register_features(feature_register_t &reg, feature_config_t const &conf) {
    }


    /*
     * Commands
     */

    std::vector<instance> create_instances(std::istream &sin) {
      auto v = read_json(sin);
      return make_vector(v["sentences"].array_items(), make_instance);
    }


    int train(docopt_t const &args) {
      //set up configuration from command-line arguments
      auto conf = make_config(args);

      //set up features
      auto reg = feature_register_t{};
      register_features(reg, conf.features);

      //read in training file (JSON) and create instances
      std::istream * sin = &std::cin;
      std::ifstream fin{};
      if (conf.input_file != "-") fin.open(conf.input_file);

      auto instances = create_instances(*sin);

      //generate features for each instance
      auto features = make_vector(instances, [&](auto const &i) {
          auto fs = reg(i);
          ranges::sort(fs);
          return fs;
        });

      //train weights using averaged perceptron

      //save weights and features

      return 0;
    }

    int test(docopt_t const &/*args*/) { return 0; }
    int run(docopt_t const &/*args*/) { return 0; }

  } //namespace classifier
} //namespace foo
