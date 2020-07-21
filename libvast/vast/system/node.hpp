/******************************************************************************
 *                    _   _____   __________                                  *
 *                   | | / / _ | / __/_  __/     Visibility                   *
 *                   | |/ / __ |_\ \  / /          Across                     *
 *                   |___/_/ |_/___/ /_/       Space and Time                 *
 *                                                                            *
 * This file is part of VAST. It is subject to the license terms in the       *
 * LICENSE file found in the top-level directory of this distribution and at  *
 * http://vast.io/license. No part of VAST, including this file, may be       *
 * copied, modified, propagated, or distributed except according to the terms *
 * contained in the LICENSE file.                                             *
 ******************************************************************************/

#pragma once

#include "vast/aliases.hpp"
#include "vast/command.hpp"
#include "vast/error.hpp"
#include "vast/filesystem.hpp"
#include "vast/system/archive.hpp"
#include "vast/system/spawn_arguments.hpp"
#include "vast/system/type_registry.hpp"

#include <caf/actor.hpp>
#include <caf/event_based_actor.hpp>
#include <caf/stateful_actor.hpp>

#include <map>
#include <string>

namespace vast::system {

struct node_state;

using node_actor = caf::stateful_actor<node_state>;

/// The state per component.
/// @relates registry.
struct component {
  caf::actor actor;
  std::string label;
};

/// Tracks all registered components.
struct component_registry {
  /// Maps component types to their state.
  std::multimap<std::string, component> components;
};

/// @relates registry
template <class Inspector>
auto inspect(Inspector& f, registry& x) {
  return f(caf::meta::type_name("registry"), x.components);
}

/// State of the node actor.
struct node_state {
  // -- member types ----------------------------------------------------------

  /// Spawns a component (actor) for the NODE with given spawn arguments.
  using component_factory_fun = maybe_actor (*)(node_actor*, spawn_arguments&);

  /// Maps command names to a component factory.
  using named_component_factory = std::map<std::string, component_factory_fun>;

  // -- static member functions ------------------------------------------------

  static caf::message
  spawn_command(const invocation& inv, caf::actor_system& sys);

  // -- constructors, destructors, and assignment operators --------------------

  node_state(caf::event_based_actor* selfptr);

  void init(std::string init_name, path init_dir);

  // -- member variables -------------------------------------------------------

  /// Stores the base directory for persistent state.
  path dir;

  /// Stores how many components per label are active.
  std::map<std::string, size_t> labels;

  /// Gives the actor a recognizable name in log files.
  std::string name = "node";

  /// Points to the node itself.
  caf::event_based_actor* self;

  /// Handle to the schema-store module.
  type_registry_type type_registry;

  /// The component registry
  vast::system::component_registry registry;

  /// Maps command names (including parent command) to spawn functions.
  inline static named_component_factory component_factory = {};

  /// Optionally creates extra component mappings.
  inline static named_component_factory (*extra_component_factory)() = nullptr;

  /// Maps command names to functions.
  inline static command::factory command_factory = {};

  /// Optionally creates extra component mappings.
  inline static command::factory (*extra_command_factory)() = nullptr;
};

/// Spawns a node.
/// @param self The actor handle
/// @param id The unique ID of the node.
/// @param dir The directory where to store persistent state.
caf::behavior node(node_actor* self, std::string id, path dir);

} // namespace vast::system
