#ifndef VAST_CONCEPT_STATE_UTIL_NONE_H
#define VAST_CONCEPT_STATE_UTIL_NONE_H

#include "vast/util/none.h"

namespace vast {

template <>
struct access::state<util::none>
{
  template <typename T, typename F>
  static void call(T&&, F)
  {
    // nop
  }
};

} // namespace vast

#endif
