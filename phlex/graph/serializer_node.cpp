#include "phlex/graph/serializer_node.hpp"
#include <oneapi/tbb/flow_graph.h>
#include <string>

namespace phlex::experimental {
  serializers::serializers(tbb::flow::graph& g) : graph_{g} {}

  serializer_node& serializers::get(std::string const& name)
  {
    return serializers_.try_emplace(name, serializer_node{graph_, name}).first->second;
  }

  void serializers::activate()
  {
    for (auto& [name, serializer] : serializers_) {
      serializer.activate();
    }
  }
}
