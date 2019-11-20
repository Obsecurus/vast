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

#define SUITE flat_map

#include "vast/detail/flat_map.hpp"

#include "vast/test/test.hpp"

using namespace vast;

namespace {

struct fixture {
  fixture() {
    xs[43] = 4.3;
    xs.insert({42, 4.2});
    xs.emplace(44, 4.4);
  }

  detail::flat_map<int, double> xs;
};

} // namespace

FIXTURE_SCOPE(flat_map_tests, fixture)

TEST(membership) {
  CHECK(xs.find(7) == xs.end());
  CHECK(xs.find(42) != xs.end());
  CHECK_EQUAL(xs.count(43), 1u);
}

TEST(insert) {
  auto i = xs.emplace(1, 3.14);
  CHECK(i.second);
  CHECK_EQUAL(i.first->first, 1);
  CHECK_EQUAL(i.first->second, 3.14);
  CHECK_EQUAL(xs.size(), 4u);
}

TEST(duplicates) {
  auto i = xs.emplace(42, 4.2);
  CHECK(!i.second);
  CHECK_EQUAL(i.first->second, 4.2);
  CHECK_EQUAL(xs.size(), 3u);
}

TEST(erase) {
  CHECK_EQUAL(xs.erase(1337), 0u);
  CHECK_EQUAL(xs.erase(42), 1u);
  REQUIRE_EQUAL(xs.size(), 2u);
  CHECK_EQUAL(xs.begin()->second, 4.3);
  CHECK_EQUAL(xs.rbegin()->second, 4.4);
  auto last = xs.erase(xs.begin());
  REQUIRE(last < xs.end());
  CHECK_EQUAL(last->first, 44);
}

FIXTURE_SCOPE_END()
