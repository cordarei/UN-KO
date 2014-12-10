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

    enum class classifier_type_t {
      binary,
      multiclass
    };

    enum class update_t {
      default_update,
      balance,
      random,
      edges
    };

    enum class output_format_t { json, svm };

    struct config_t {
      feature_config_t features;
      classifier_type_t classifier;
      update_t update;
      output_format_t output_format;
      bool parse_oracle;
      std::string input_file;
      std::string output_file; //optional
      std::string model_file;
      std::string weights_file;
      std::string feature_file;
    };

    inline
    config_t make_config(docopt_t const &args) {
      auto conf = config_t{};
      conf.classifier = classifier_type_t::binary;
      conf.update = update_t::default_update;
      conf.output_format = output_format_t::json;

      // auto upd = check_docopt_arg<std::string>(args, "--update");
      // if (upd) {
      //   auto &s = *upd;
      //   if (s != "binary") {
      //     throw std::runtime_error("multiclass not implemented");
      //   }
      // }

      conf.input_file = *check_docopt_arg<std::string>(args, "--input");

      auto model = *check_docopt_arg<std::string>(args, "--model");
      conf.model_file = model;
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

      if (check_docopt_flag(args, "--svm-format")) {
        conf.output_format = output_format_t::svm;
      }

      conf.parse_oracle = check_docopt_flag(args, "--oracle");

      return conf;
    }

  }
}


#endif
