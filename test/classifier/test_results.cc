#include <catch/catch.hpp>

#include "../../src/classifier/results.h"

#include <range/v3/algorithm/for_each.hpp>


TEST_CASE("") {
  using foo::classifier::classification_results;
  constexpr auto tp = classification_results::result_t::tp;
  constexpr auto fp = classification_results::result_t::fp;
  constexpr auto tn = classification_results::result_t::tn;
  constexpr auto fn = classification_results::result_t::fn;

  auto res = classification_results{};

  auto xs = {tp, tp, fp, tp, tn, tn, tn, tn, tn, fn};
  ranges::for_each(xs, [&res](auto v) { res.add(v); });

  REQUIRE(res.tp() == 3);
  REQUIRE(res.fp() == 1);
  REQUIRE(res.tn() == 5);
  REQUIRE(res.fn() == 1);
  REQUIRE(res.accuracy() == 0.8f);
  REQUIRE(res.recall() == 0.75f);
  REQUIRE(res.precision() == 0.75f);
  REQUIRE(res.f1() == 0.75f);
}
