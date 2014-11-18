#include "instance.h"

#include <experimental/optional>

#include <foo/types.h>
#include <foo/span.h>
#include <foo/utility/container.h>
#include <json11/json11.hpp>
#include <range/v3/algorithm/any_of.hpp>


namespace foo {
  namespace classifier {

    sentence_t make_sentence(json const &j) {
      using std::experimental::optional;
      using std::experimental::nullopt;
      auto words = make_vector(j["words"].array_items(), &json::string_value);
      auto tags  = make_vector(j["tags" ].array_items(), &json::string_value);
      auto ans = std::experimental::optional<answer_t>{};
      if (!j["parse"].is_null()) {
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
        auto split_points = make_vector(top_spans, &span_t::end);
        split_points.erase(--split_points.end());

        ans = std::experimental::optional<answer_t>{{
            std::move(top_spans),
            std::move(split_points)
          }};
      }
      return { std::move(words), std::move(tags), std::move(ans) };
    }

    bool is_legal(instance_t const &i) {
      auto & ans = i.sentence().answer;
      auto sp = i.sp();
      if (!ans) return false;
      auto & legal = (*ans).legal_split_points;
      return ranges::any_of(legal, [&sp](auto j) { return j == sp; });
    }

  }
}
