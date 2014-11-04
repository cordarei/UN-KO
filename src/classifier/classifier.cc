#include "classifier.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include <foo/utility/container.h>

#include "instance.h"


namespace foo {
namespace classifier {

std::vector<instance> create_instances(std::istream &sin) {
  auto v = read_json(sin);
  return make_vector(v["sentences"].array_items(), make_instance);
}

int train() {
  //set up configuration from command-line arguments

  //set up features

  //read in training file (JSON) and create instances
  auto instances = create_instances(std::cin);

  //generate features for each instance

  //train weights using averaged perceptron

  //save weights and features

  return 0;
}

int test() { return 0; }
int run() { return 0; }

} //namespace classifier
} //namespace foo
