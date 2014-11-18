#include <catch/catch.hpp>

#include <foo/features.h>

#include <string>
#include <vector>
#include <sstream>


SCENARIO("a feature registry can be created and used") {
  using foo::feature_registry_t;
  using foo::feature_id_t;
  using bigram_t = std::string;
  auto tags = std::vector<std::string>{"A", "B", "C"};
  auto bigrams = [](auto && ts) {
    auto bs = std::vector<bigram_t>{};
    auto bos = std::string{"^"};
    auto const * last = &bos;
    for(auto const& t : ts) {
      bs.emplace_back(*last + t);
      last = &t;
    }
    bs.emplace_back(*last + "$");
    return bs;
  };
  auto feature_reg = feature_registry_t<std::vector<std::string>, bigram_t>{};
  feature_reg.add_feature(bigrams);
  auto fs = feature_reg(tags);

  WHEN("the example function is used") {
    auto bs = bigrams(tags);
    THEN("it returns all the bigrams") {
      REQUIRE(bigrams(tags) == (std::vector<bigram_t>{"^A", "AB", "BC", "C$"}));
    }
  }

  WHEN("creating features from a registry with one function") {
    THEN("the feature ids are consecutive starting at 1") {
      REQUIRE(fs == (std::vector<feature_id_t>{1, 2, 3, 4}));
    }
  }

  WHEN("creating features from the same instance twice") {
    auto fs2 = feature_reg(tags);
    THEN("the feature ids are the same") {
      REQUIRE(fs2 == fs);
    }
  }

  WHEN("saving and loading a feature registry") {
    //auto ss = std::stringstream{}; //gcc libstdc++ streams not movable yet
    std::stringstream ss;
    feature_reg.save(ss);
    auto reg2 = feature_registry_t<std::vector<std::string>>{};
    reg2.load(ss);
    reg2.add_feature(bigrams); //need to re-add feature functions
    auto ts2 = std::vector<std::string>{"C", "A", "B"};
    auto fs2 = reg2(ts2);
    THEN("the old and new feature ids are combined") {
      REQUIRE(fs2 == (std::vector<feature_id_t>{5, 6, 2, 7}));
    }
  }

  WHEN("creating features through a const registry") {
    auto const & reg = feature_reg;
    auto ts2 = std::vector<std::string>{"C", "A", "B"};
    auto fs2 = reg(ts2);
    THEN("unseen features are not added") {
      REQUIRE(fs2 == (std::vector<feature_id_t>{2}));
    }
  }

}
