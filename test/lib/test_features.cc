#include <catch/catch.hpp>

#include <functional>
#include <string>
#include <vector>
#include <iosfwd>
#include <sstream>


using feature_id_t = size_t;

template <typename Instance, typename FV = std::string>
struct feature_registry_t {
  using instance_type = Instance;
  using feature_value_type = FV;
  using feature_function_return_type = std::vector<feature_value_type>;
  using feature_function_type = std::function<feature_function_return_type(instance_type const &)>;
  using return_type = std::vector<feature_id_t>;

private:
  std::vector<std::pair<feature_value_type, feature_id_t>> id_map_;
  std::vector<feature_function_type> ffs_;

public:
  feature_registry_t() = default;
  ~feature_registry_t() = default;
  feature_registry_t(feature_registry_t const &) = delete;
  feature_registry_t(feature_registry_t &&) = default;
  feature_registry_t & operator=(feature_registry_t const &) = delete;
  feature_registry_t & operator=(feature_registry_t &&) = default;

  void add_feature(feature_function_type ff) {}

  return_type operator()(instance_type const &) {
    return {};
  }

  void save(std::ostream &) {}
  void load(std::istream &) {}
};


SCENARIO("a feature registry can be created and used" "[features]") {
  using bigram_t = std::pair<std::string, std::string>;
  auto tags = std::vector<std::string>{"A", "B", "C"};
  auto bigrams = [](auto && ts) {
    auto bs = std::vector<bigram_t>{};
    auto bos = std::string{"^"};
    auto const * last = &bos;
    for(auto const& t : ts) {
      bs.emplace_back(*last, t);
      last = &t;
    }
    bs.emplace_back(*last, "$");
    return bs;
  };
  auto feature_reg = feature_registry_t<std::vector<std::string>, bigram_t>{};
  feature_reg.add_feature(bigrams);

  WHEN("the example function is used") {
    auto bs = bigrams(tags);
    THEN("it returns all the bigrams") {
      REQUIRE(bigrams(tags) == (std::vector<bigram_t>{{"^","A"}, {"A", "B"}, {"B", "C"}, {"C", "$"}}));
    }
  }

  WHEN("creating features from a registry with one function") {
    auto fs = feature_reg(tags);
    THEN("the feature ids are consecutive starting at 1") {
      REQUIRE(fs == (std::vector<feature_id_t>{1, 2, 3, 4}));
    }
  }

  WHEN("creating features from the same instance twice") {
    auto fs = feature_reg(tags);
    auto fs2 = feature_reg(tags);
    THEN("the feature ids are the same") {
      REQUIRE(fs2 == fs);
    }
  }

  WHEN("saving and loading a feature registry") {
    //auto ss = std::stringstream{}; //gcc libstdc++ streams not movable yet
    std::stringstream ss;
    auto fs = feature_reg(tags);
    feature_reg.save(ss);
    auto reg2 = feature_registry_t<std::vector<std::string>>{};
    reg2.load(ss);
    auto ts2 = std::vector<std::string>{"C", "A", "B"};
    auto fs2 = reg2(ts2);
    THEN("the old and new feature ids are combined") {
      REQUIRE(fs2 == (std::vector<feature_id_t>{5, 6, 2, 7}));
    }
  }

}
