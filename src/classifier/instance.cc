#include "instance.h"

#include <experimental/optional>

#include <foo/types.h>
#include <foo/span.h>
#include <foo/utility/container.h>
#include <json11/json11.hpp>


namespace foo {
namespace classifier {

  instance make_instance(json const &j) {
    using std::experimental::optional;
    using std::experimental::nullopt;
    auto words = make_vector(j["words"].array_items(), &json::string_value);
    auto tags  = make_vector(j["tags" ].array_items(), &json::string_value);
    auto top_span_idx = make_vector(j["parse"]["edges"][1][1].array_items(), &json::int_value);
    auto top_spans = make_vector(top_span_idx, [&](int i) {
        auto &&k = j["parse"]["spans"][i];
        auto b = static_cast<offset_t>(k[1].int_value());
        auto e = static_cast<offset_t>(k[2].int_value());
        auto l = k[0].is_string() ? optional<symbol_t>{k[0].string_value()} : nullopt;
        if (l) {
          return span_t{*l, b, e};
        } else {
          return span_t{tags[b], b, e};
        }
      });
    return { std::move(words), std::move(tags), std::move(top_spans) };
  }

}
}
