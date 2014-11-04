#ifndef FOO_INCLUDE_UTILITY_JOIN_H
#define FOO_INCLUDE_UTILITY_JOIN_H


#include <range/v3/utility/invokable.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/range_traits.hpp>
#include <range/v3/empty.hpp>
#include <range/v3/front.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/algorithm/for_each.hpp>

namespace foo {

  template<
    typename Rng,
    typename P = ranges::ident,
    typename V1 = ranges::range_value_t<Rng>,
    typename V2 = decltype(ranges::invokable(std::declval<P>())(std::declval<V1>())),
    CONCEPT_REQUIRES_(ranges::InputIterable<Rng>() &&
                      ranges::Invokable<P, V1>())
    >
    std::string join(Rng &&r, std::string delim = "", P proj_ = P{}) {
      std::string joined;
      if (!ranges::empty(r)) {
        auto && proj = ranges::invokable(proj_);
        joined = proj(ranges::front(r));
        auto rr = r | ranges::view::drop(1);
        ranges::for_each(rr, [&](auto &&s) {
          joined += delim;
          joined += proj(s);
        });
      }
      return joined;
    }
}


#endif
