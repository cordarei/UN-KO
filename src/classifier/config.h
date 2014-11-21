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
    config_t make_config(docopt_t const &/*args*/) {
      return {};
    }

  }
}


#endif
