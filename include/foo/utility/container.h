#ifndef FOO_INCLUDE_CONTAINER_H
#define FOO_INCLUDE_CONTAINER_H

#include <vector>

#include <range/v3/utility/invokable.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/view/transform.hpp>

namespace foo {

template<typename Rng,
         typename P = ranges::ident,
         typename V = std::decay_t<
           ranges::concepts::Invokable::result_t<P, ranges::range_value_t<Rng>>
           >,
         CONCEPT_REQUIRES_(ranges::InputIterable<Rng>() &&
                           ranges::Invokable<P, ranges::range_value_t<Rng>>())>
std::vector<V> make_vector(Rng && rng, P proj = P{}) {
  auto transformed = ranges::view::transform(rng, std::move(proj));
  return transformed;
}

}

#endif
