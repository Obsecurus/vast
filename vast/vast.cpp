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

#include <caf/actor_system.hpp>

#include "vast/logger.hpp"

#include "vast/system/application.hpp"
#include "vast/system/configuration.hpp"
#include "vast/system/run_import.hpp"
#include "vast/system/run_start.hpp"

using namespace vast;

int main(int argc, char** argv) {
  VAST_TRACE("");
  system::configuration cfg{argc, argv};
  caf::actor_system sys{cfg};
  system::application app;
  app.add_command<system::run_start>("start");
  app.add_command<system::run_import>("import");
  return app.run(sys,
                 caf::message_builder{cfg.command_line.begin(),
                                      cfg.command_line.end()}
                 .move_to_message());
}
