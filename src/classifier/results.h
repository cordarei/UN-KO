#ifndef FOO_SRC_CLASSIFIER_RESULTS_H
#define FOO_SRC_CLASSIFIER_RESULTS_H


#include <array>

#include <range/v3/numeric/accumulate.hpp>


namespace foo {
  namespace classifier {

    struct classification_results {
    public:
      enum class result_t { tp, fp, tn, fn };
      using count_t = size_t;

    private:
      std::array<count_t, 4> counts_;

    public:
      classification_results() : counts_{{0}} {}
      ~classification_results() = default;
      classification_results(classification_results const &) = default;
      classification_results(classification_results &&) = default;
      classification_results & operator=(classification_results const &) = default;
      classification_results & operator=(classification_results &&) = default;

      void add(result_t v) {
        ++counts_[static_cast<size_t>(v)];
      }

      count_t tp() const { return counts_[static_cast<count_t>(result_t::tp)]; }
      count_t fp() const { return counts_[static_cast<count_t>(result_t::fp)]; }
      count_t tn() const { return counts_[static_cast<count_t>(result_t::tn)]; }
      count_t fn() const { return counts_[static_cast<count_t>(result_t::fn)]; }

      float accuracy() const {
        return static_cast<float>(tp() + tn()) / ranges::accumulate(counts_, 0u);
      }

      float recall() const {
        return tp() / static_cast<float>(tp() + fn());
      }

      float precision() const {
        return tp() / static_cast<float>(tp() + fp());
      }

      float f1() const {
        auto r = recall();
        auto p = precision();
        return 2 * r * p / (r + p);
      }
    };

  }
}


#endif
