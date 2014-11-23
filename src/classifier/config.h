#ifndef FOO_SRC_CLASSIFIER_CONFIG_H
#define FOO_SRC_CLASSIFIER_CONFIG_H


#include <string>
#include "../common.h"


namespace foo {
  namespace classifier {

    struct feature_config_t {
      bool pos = false;
      bool word = false;
      bool global = false;
      bool local = false;
    };

    enum class update_t {
      binary,
      default_multiclass,
      balance,
      random,
      edges
    };

    struct config_t {
      feature_config_t features;
      update_t update;
      std::string input_file;
      std::string output_file; //optional
      std::string weights_file;
      std::string feature_file;
    };

    inline
    config_t make_config(docopt_t const &args) {
      auto conf = config_t{};
      conf.update = update_t::binary;

      auto upd = check_docopt_arg<std::string>(args, "--update");
      if (upd) {
        auto &s = *upd;
        if (s != "binary") {
          throw std::runtime_error("multiclass not implemented");
        }
      }

      conf.input_file = *check_docopt_arg<std::string>(args, "--input");

      auto model = *check_docopt_arg<std::string>(args, "--model");
      conf.weights_file = model + ".weights";
      conf.feature_file = model + ".features";

      auto out = check_docopt_arg<std::string>(args, "--output");
      if (out) {
        conf.output_file = *out;
      }

      conf.features.pos = check_docopt_flag(args, "--feat-pos");
      conf.features.word = check_docopt_flag(args, "--feat-word");
      conf.features.global = check_docopt_flag(args, "--feat-global");
      conf.features.local = check_docopt_flag(args, "--feat-local");

      return conf;
    }

  }
}


#endif
