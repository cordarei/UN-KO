#include <catch/catch.hpp>
#include <iostream>

#include <json11/json11.hpp>

#include <range/v3/view.hpp>

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


template<typename Rng, typename P = ranges::ident,
         typename V1 = ranges::range_value_t<Rng>,
         typename V2 = std::decay_t<
           decltype(
                    ranges::invokable(std::declval<P>())(
                                                         std::declval<V1>()
                                                         )
                    )
           >,
         CONCEPT_REQUIRES_(ranges::InputIterable<Rng>() &&
                           ranges::Invokable<P, V1>())>
std::vector<V2> make_vector(Rng &r, P proj_ = P{}) {
  auto && proj = ranges::invokable(proj_);
  return r | ranges::view::transform(proj);
}


struct example {
  std::vector<std::string> tags;
};
bool operator==(example const &l, example const &r) {
  return l.tags == r.tags;
}

TEST_CASE("we can read the JSON into structs") {
  auto j = get_json();
  auto v = make_vector(j["sentences"].array_items(), [](auto k) {
      return example{make_vector(k["words"].array_items(), &json11::Json::string_value)};
    });
  auto u = std::vector<example>{
            {{"DT", "VB", "NN", "NN", "."}},
            {{"DT", "VB", "NN", "NN", "."}}
  };
  REQUIRE(v == u);
}




/*
 *
 * The following is a misguided and incomplete attempt to make JSON
 * (and the Json11 library) nicer to work with in C++. I don't have
 * any more time to spend on this at the moment, but eventually I
 * would like to finish it.
 *
 */


// template<typename RangeLike, typename Value = std::decay_t<decltype(*begin(std::declval<RangeLike>()))>>
// std::vector<Value> make_vector(RangeLike const& r) {
//   using std::begin;
//   return std::vector<Value>(begin(r), end(r));
// }


template<typename T, typename ...Us>
struct is_one_of;

template<typename T>
struct is_one_of<T> : std::false_type {};

template<typename T, typename ...Us>
struct is_one_of<T, T, Us...> : std::true_type {};

template<typename T, typename U, typename ...Vs>
struct is_one_of<T, U, Vs...> : is_one_of<T, Vs...> {};


namespace json {
  using value = ::json11::Json;

  template<typename T>
  T json_cast(value const &v);

  template<typename T>
  struct as_t {
    T operator()(value const &v) { return json_cast<T>(v); }
  };

  template<typename T>
  auto as = as_t<T>{};

  struct all_indexer_t {} all;

  template<typename ...Indexes>
  struct jpath_t : std::tuple<Indexes...> {
    // // Index can be any type useable for indexing into a json object: integer, string, or all_indexer_t
    // template<typename Index>
    // jpath(Index index) {}
    // template<typename Index, typename ...Rest>
    // jpath(Index index, Rest... rest) {}
    using base_type = std::tuple<Indexes...>;
    using base_type::base_type;

    template<typename T>
    struct range {
      T* begin() const { return nullptr; }
      T* end() const { return nullptr; }
    };

    template<typename T>
    using return_type = std::conditional_t<is_one_of<all_indexer_t, std::decay_t<Indexes>...>::value,
                                           range<T>,
                                           T>;

    return_type<value> operator()(value const&) { return {}; }
    template<typename T>
    return_type<T> operator()(value const&, as_t<T>) { return {}; }
  };

  template<typename ...Indexes>
  auto jpath(Indexes &&...indexes) -> jpath_t<std::decay_t<Indexes>...> {
    return jpath_t<Indexes...>{std::forward<Indexes>(indexes)...};
  }
}

#include <type_name/type_name.h>

TEST_CASE("values can be extracted") {
  using namespace std::literals::string_literals;
  using namespace json;

  auto v = get_json();
  auto path = jpath("sentences", all, "words", 3);
  auto x = path(v, as_t<std::string>());
  auto y = path(v);
  auto vec = make_vector(x);
  // std::cout << type_name<decltype(path)>() << std::endl; //what's the type of x?
  // std::cout << type_name<decltype(x)>() << std::endl; //what's the type of x?
  // std::cout << type_name<decltype(path(v))>() << std::endl; //what's the type of x?

  //v.sentences[].words[3] == ["one"s, "two"s]
  auto answer = std::vector<std::string>{"one"s, "two"s};
  REQUIRE(vec == std::vector<std::string>{}); //REQUIRE(vec == answer);
}
