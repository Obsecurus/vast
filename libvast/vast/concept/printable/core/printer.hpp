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

#ifndef VAST_CONCEPT_PRINTABLE_CORE_PRINTER_HPP
#define VAST_CONCEPT_PRINTABLE_CORE_PRINTER_HPP

#include <type_traits>
#include <iterator>
#include <tuple>

#include "vast/concept/support/unused_type.hpp"

namespace vast {

template <typename, typename>
class action_printer;

template <typename, typename>
class guard_printer;

template <typename Derived>
struct printer {
  template <typename Action>
  auto before(Action fun) const {
    return action_printer<Derived, Action>{derived(), fun};
  }

  template <typename Action>
  auto operator->*(Action fun) const {
    return before(fun);
  }

  template <typename Guard>
  auto with(Guard fun) const {
    return guard_printer<Derived, Guard>{derived(), fun};
  }

  // FIXME: don't ignore ADL.
  template <typename Range, typename Attribute = unused_type>
  auto operator()(Range&& r, const Attribute& a = unused) const
  -> decltype(std::begin(r), std::end(r), bool()) {
    auto out = std::back_inserter(r);
    return derived().print(out, a);
  }

  // FIXME: don't ignore ADL.
  template <typename Range, typename A0, typename A1, typename... As>
  auto operator()(Range&& r, const A0& a0, const A1& a1, const As&... as) const
  -> decltype(std::begin(r), std::end(r), bool()) {
    return operator()(r, std::tie(a0, a1, as...));
  }

  template <typename Iterator, typename Attribute = unused_type>
  auto operator()(Iterator&& out, const Attribute& a = unused) const
  -> decltype(*out, ++out, bool()) {
    return derived().print(out, a);
  }

  template <typename Iterator, typename A0, typename A1, typename... As>
  auto operator()(Iterator&& out, const A0& a0, const A1& a1, const As&... as) const
  -> decltype(*out, ++out, bool()) {
    return operator()(out, std::tie(a0, a1, as...));
  }

private:
  const Derived& derived() const {
    return static_cast<const Derived&>(*this);
  }
};

/// Associates a printer for a given type. To register a printer with a type,
/// one needs to specialize this struct and expose a member `type` with the
/// concrete printer type.
/// @tparam T the type to register a printer with.
template <typename T, typename = void>
struct printer_registry;

/// Retrieves a registered printer.
template <typename T>
using make_printer = typename printer_registry<T>::type;

namespace detail {

struct has_printer {
  template <typename T>
  static auto test(T* x)
    -> decltype(typename printer_registry<T>::type(), std::true_type());

  template <typename>
  static auto test(...) -> std::false_type;
};

} // namespace detail

/// Checks whether the printer registry has a given type registered.
template <typename T>
struct has_printer : decltype(detail::has_printer::test<T>(0)) {};

/// Checks whether a given type is-a printer, i.e., derived from
/// ::vast::printer.
template <typename T>
using is_printer = std::is_base_of<printer<T>, T>;

} // namespace vast

#endif
