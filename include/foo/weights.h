#ifndef FOO_INCLUDE_FOO_WEIGHTS_H
#define FOO_INCLUDE_FOO_WEIGHTS_H


#include <vector>

#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/take.hpp>

#include "features.h"


namespace foo {

  struct weights_t {
  private:
    std::vector<double> w_;

  public:
    weights_t(size_t max_id)
      : w_(max_id + 1, 0.0)
    {}

    weights_t() = default;
    ~weights_t() = default;
    weights_t(weights_t const &) = delete;
    weights_t(weights_t &&) = default;
    weights_t & operator=(weights_t const &) = delete;
    weights_t & operator=(weights_t &&) = default;

    double score(std::vector<feature_id_t> const & fs) {
      auto & w = w_;
      return ranges::accumulate(fs, 0.0, ranges::plus{}, [&w](auto id) { return w[id]; });
    }

    void update(std::vector<feature_id_t> const & fs, double val) {
      auto & w = w_;
      for (auto && id : fs) {
        w[id] += val;
      }
    }

    void update(weights_t const & u, double val) {
      auto & w = this->w_;
      auto & uw = u.w_;
      for (auto i : ranges::view::iota(0u) | ranges::view::take(uw.size())) {
        w[i] += uw[i] * val;
      }
    }
  };
}


#endif
