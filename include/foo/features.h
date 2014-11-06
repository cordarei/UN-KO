#ifndef FOO_INCLUDE_FOO_FEATURES_H
#define FOO_INCLUDE_FOO_FEATURES_H


#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <range/v3/lines_range.hpp>
#include <range/v3/range_for.hpp>


namespace foo {
  using feature_id_t = size_t;

  template <typename Instance, typename FeatureValue = std::string>
  struct feature_registry_t {
    using instance_type = Instance;
    using feature_value_type = FeatureValue;
    using feature_function_return_type = std::vector<feature_value_type>;
    using feature_function_type = std::function<feature_function_return_type(instance_type const &)>;
    using return_type = std::vector<feature_id_t>;

  private:
    using feature_map_t = std::unordered_map<feature_value_type, feature_id_t>;
    feature_map_t map_;
    std::vector<feature_function_type> ffs_;
    feature_id_t next_id_ = {1u};

  public:
    feature_registry_t() = default;
    ~feature_registry_t() = default;
    feature_registry_t(feature_registry_t const &) = delete;
    feature_registry_t(feature_registry_t &&) = default;
    feature_registry_t & operator=(feature_registry_t const &) = delete;
    feature_registry_t & operator=(feature_registry_t &&) = default;

    void add_feature(feature_function_type ff) { ffs_.push_back(std::move(ff)); }

    return_type operator()(instance_type const &instance) {
      auto feature_ids = return_type{};

      for(auto && ff : ffs_) {
        auto && fs = ff(instance);
        for(auto && fv : fs) {
          auto it = map_.find(fv);
          if (it != std::end(map_)) {
            feature_ids.push_back(it->second);
          } else {
            auto id = next_id_++;
            assert(id < next_id_);
            map_.emplace(std::move(fv), id);
            feature_ids.push_back(id);
          }
        }
      }

      return feature_ids;
    }

    return_type operator()(instance_type const &instance) const {
      auto feature_ids = return_type{};

      for(auto && ff : ffs_) {
        auto && fs = ff(instance);
        for(auto && fv : fs) {
          auto it = map_.find(fv);
          if (it != std::end(map_)) {
            feature_ids.push_back(it->second);
          }
        }
      }

      return feature_ids;
    }

    void save(std::ostream &sout) {
      feature_value_type fv;
      feature_id_t id;
      for(auto && p : map_) {
        std::tie(fv, id) = p;
        sout << fv << "\t" << id << std::endl;
      }
    }

    void load(std::istream &sin) {
      // should we prevent loading into a feature_registry_t that already contains features?
      RANGES_FOR(auto && line, ranges::lines(sin)) {
        auto tabpos = line.find('\t');
        assert(tabpos != std::string::npos);
        auto tmp = stoi(line.substr(tabpos+1));
        assert(tmp >= 0);
        auto id = static_cast<feature_id_t>(tmp);
        next_id_ = std::max(next_id_, id + 1);
        map_.emplace(line.substr(0, tabpos), id);
      }
    }
  };
}


#endif
