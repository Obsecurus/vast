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

#include <vector>

#include "vast/fwd.hpp"
#include "vast/table_slice.hpp"

namespace vast {

/// The default implementation of `table_slice`.
class default_table_slice final : public table_slice {
public:
  // -- friends ----------------------------------------------------------------

  friend default_table_slice_builder;

  // -- constructors, destructors, and assignment operators --------------------

  default_table_slice(record_type layout);

  // -- static factory functions -----------------------------------------------

  /// Constructs a builder that generates a default_table_slice.
  /// @param layout The layout of the table_slice.
  /// @returns The builder instance.
  static table_slice_builder_ptr make_builder(record_type layout);

  // -- properties -------------------------------------------------------------

  caf::optional<data_view> at(size_type row, size_type col) const final;

private:
  // -- member variables -------------------------------------------------------

  std::vector<data> xs_;
};

/// @relates default_table_slice
using default_table_slice_ptr = caf::intrusive_ptr<default_table_slice>;

} // namespace vast