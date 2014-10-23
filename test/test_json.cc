#include <catch/catch.hpp>

#include <json11/json11.hpp>

constexpr auto JSON_STRING =
  u8R"({"sentences" : [
  {
    "words" : ["This", "is", "sentence", "one", "."],
    "tags" : ["DT", "VB", "NN", "NN", "."],
    "parse" : {
      "spans" : [["ROOT", 0, 5], ["S", 0, 5], ["NP", 0, 1], [null, 0, 1], ["VP", 1, 4], [null, 1, 2], ["NP", 2, 4], [null, 2, 3], [null, 3, 4], [null, 4, 5]],
      "edges" : [[0, [1]], [1, [2, 4, 9]], [2, [3]], [4, [5, 6]], [6, [7, 8]]]
    }
  },
  {
    "words" : ["This", "is", "sentence", "two", "."],
    "tags" : ["DT", "VB", "NN", "NN", "."],
    "parse" : {
      "spans" : [["ROOT", 0, 5], ["S", 0, 5], ["NP", 0, 1], [null, 0, 1], ["VP", 1, 4], [null, 1, 2], ["NP", 2, 4], [null, 2, 3], [null, 3, 4], [null, 4, 5]],
      "edges" : [[0, [1]], [1, [2, 4, 9]], [2, [3]], [4, [5, 6]], [6, [7, 8]]]
    }
  }
]
})";

json11::Json get_json() {
  std::string err;
  return json11::Json::parse(JSON_STRING, err);
}


TEST_CASE("we can parse the JSON") {
  auto v = get_json();
  REQUIRE(v["sentences"][0]["words"][3].string_value() == "one");
  REQUIRE(v["sentences"][1]["words"][3].string_value() == "two");
}


using std::begin;

template<typename RangeLike>
struct begin_iterator_type {
  typedef decltype(begin(std::declval<RangeLike>())) type;
};
template<typename RangeLike>
struct value_type {
  typedef decltype(*std::declval<typename begin_iterator_type<RangeLike>::type>()) dereferenced_type;
  typedef typename std::remove_reference<dereferenced_type>::type type;
};

template<typename RangeLike, typename Value = typename value_type<RangeLike>::type>
std::vector<Value> make_vector(RangeLike const& r) {
  return std::vector<Value>(begin(r), end(r));
}




namespace json {
  using value = ::json11::Json;

  template<typename T>
  T json_cast(value const &v);

  template<typename T>
  struct as_t {
    T operator()(value const &v) { return json_cast<T>(v); }
  };

  // template<typename T>
  // auto as = as_t<T>{};

  struct all_indexer_t {} all;

  struct jpath {
    jpath() {}
    // Index can be any type useable for indexing into a json object: integer, string, or all_indexer_t
    template<typename Index>
    jpath(Index index) {}
    template<typename Index, typename ...Rest>
    jpath(Index index, Rest... rest) {}

    value operator()(value const& v) { return v; }
    template<typename T>
    T operator()(value const& v, as_t<T>) { return T{}; }
  };
}


TEST_CASE("values can be extracted") {
  using namespace std::literals::string_literals;
  using namespace json;

  auto v = get_json();
  auto path = jpath("sentences", all, "words", 3);
  auto x = path(v, as_t<std::string>());
  auto vec = make_vector(x);

  //v.sentences[].words[3] == ["one"s, "two"s]
  //REQUIRE(vec == make_vector({"one"s, "two"s}));
}
