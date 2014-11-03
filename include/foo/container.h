#ifndef FOO_INCLUDE_CONTAINER_H
#define FOO_INCLUDE_CONTAINER_H

#include <vector>

#include <range/v3/utility/invokable.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/view/transform.hpp>

namespace foo {

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
std::vector<V2> make_vector(Rng &r, P proj = P{}) {
  return r | ranges::view::transform(proj);
}

}

#endif
