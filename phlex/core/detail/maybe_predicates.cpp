#include "phlex/core/detail/maybe_predicates.hpp"
#include "phlex/configuration.hpp"
#include <optional>
#include <string>
#include <vector>

namespace phlex::experimental::detail {
  std::optional<std::vector<std::string>> maybe_predicates(configuration const* config)
  {
    return config->get_if_present<std::vector<std::string>>("when");
  }
}
