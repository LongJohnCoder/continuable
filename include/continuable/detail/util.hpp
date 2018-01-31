
/*

                        /~` _  _ _|_. _     _ |_ | _
                        \_,(_)| | | || ||_|(_||_)|(/_

                    https://github.com/Naios/continuable
                                   v2.0.0

  Copyright(c) 2015 - 2018 Denis Blank <denis.blank at outlook dot com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files(the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions :

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
**/

#ifndef CONTINUABLE_DETAIL_UTIL_HPP_INCLUDED_
#define CONTINUABLE_DETAIL_UTIL_HPP_INCLUDED_

#include <cassert>
#include <tuple>
#include <type_traits>
#include <utility>

#include <continuable/continuable-api.hpp>
#include <continuable/detail/features.hpp>
#include <continuable/detail/traits.hpp>

namespace cti {
namespace detail {
/// Utility namespace which provides useful meta-programming support
namespace util {
/// Helper to trick compilers about that a parameter pack is used
template <typename... T>
void unused(T&&...) {
}

namespace detail {
template <typename T, typename Args, typename = traits::void_t<>>
struct is_invokable_impl : std::common_type<std::false_type> {};

template <typename T, typename... Args>
struct is_invokable_impl<
    T, std::tuple<Args...>,
    traits::void_t<decltype(std::declval<T>()(std::declval<Args>()...))>>
    : std::common_type<std::true_type> {};
} // namespace detail

/// Deduces to a std::true_type if the given type is callable with the arguments
/// inside the given tuple.
/// The main reason for implementing it with the detection idiom instead of
/// hana like detection is that MSVC has issues with capturing raw template
/// arguments inside lambda closures.
///
/// ```cpp
/// traits::is_invokable<object, std::tuple<Args...>>
/// ```
template <typename T, typename Args>
using is_invokable = typename detail::is_invokable_impl<T, Args>::type;

namespace detail {
/// Forwards every element in the tuple except the last one
template <typename T>
auto forward_except_last(T&& sequenceable) {
  auto size = pack_size_of(traits::identity_of(sequenceable)) -
              traits::size_constant_of<1>();
  auto sequence = std::make_index_sequence<size.value>();

  return traits::unpack(std::forward<T>(sequenceable),
                        [](auto&&... args) {
                          return std::forward_as_tuple(
                              std::forward<decltype(args)>(args)...);
                        },
                        sequence);
}

/// We are able to call the callable with the arguments given in the tuple
template <typename T, typename... Args>
auto partial_invoke_impl(std::true_type, T&& callable,
                         std::tuple<Args...> args) {
  return traits::unpack(std::move(args), [&](auto&&... arg) {
    return std::forward<T>(callable)(std::forward<decltype(arg)>(arg)...);
  });
}

/// We were unable to call the callable with the arguments in the tuple.
/// Remove the last argument from the tuple and try it again.
template <typename T, typename... Args>
auto partial_invoke_impl(std::false_type, T&& callable,
                         std::tuple<Args...> args) {

  // If you are encountering this assertion you tried to attach a callback
  // which can't accept the arguments of the continuation.
  //
  // ```cpp
  // continuable<int, int> c;
  // std::move(c).then([](std::vector<int> v) { /*...*/ })
  // ```
  static_assert(
      sizeof...(Args) > 0,
      "There is no way to call the given object with these arguments!");

  // Remove the last argument from the tuple
  auto next = forward_except_last(std::move(args));

  // Test whether we are able to call the function with the given tuple
  is_invokable<decltype(callable), decltype(next)> is_invokable;

  return partial_invoke_impl(is_invokable, std::forward<T>(callable),
                             std::move(next));
}

/// Shortcut - we can call the callable directly
template <typename T, typename... Args>
auto partial_invoke_impl_shortcut(std::true_type, T&& callable,
                                  Args&&... args) {
  return std::forward<T>(callable)(std::forward<Args>(args)...);
}

/// Failed shortcut - we were unable to invoke the callable with the
/// original arguments.
template <typename T, typename... Args>
auto partial_invoke_impl_shortcut(std::false_type failed, T&& callable,
                                  Args&&... args) {

  // Our shortcut failed, convert the arguments into a forwarding tuple
  return partial_invoke_impl(
      failed, std::forward<T>(callable),
      std::forward_as_tuple(std::forward<Args>(args)...));
}
} // namespace detail

/// Partially invokes the given callable with the given arguments.
///
/// \note This function will assert statically if there is no way to call the
///       given object with less arguments.
template <typename T, typename... Args>
/*keep this inline*/ inline auto partial_invoke(T&& callable, Args&&... args) {
  // Test whether we are able to call the function with the given arguments.
  is_invokable<decltype(callable), std::tuple<Args...>> is_invokable;

  // The implementation is done in a shortcut way so there are less
  // type instantiations needed to call the callable with its full signature.
  return detail::partial_invoke_impl_shortcut(
      is_invokable, std::forward<T>(callable), std::forward<Args>(args)...);
}

// Class for making child classes non copyable
struct non_copyable {
  constexpr non_copyable() = default;
  non_copyable(non_copyable const&) = delete;
  constexpr non_copyable(non_copyable&&) = default;
  non_copyable& operator=(non_copyable const&) = delete;
  non_copyable& operator=(non_copyable&&) = default;
};

// Class for making child classes non copyable and movable
struct non_movable {
  constexpr non_movable() = default;
  non_movable(non_movable const&) = delete;
  constexpr non_movable(non_movable&&) = delete;
  non_movable& operator=(non_movable const&) = delete;
  non_movable& operator=(non_movable&&) = delete;
};

/// This class is responsible for holding an abstract copy- and
/// move-able ownership that is invalidated when the object
/// is moved to another instance.
class ownership {
  explicit constexpr ownership(bool acquired, bool frozen)
      : acquired_(acquired), frozen_(frozen) {
  }

public:
  constexpr ownership() : acquired_(true), frozen_(false) {
  }
  constexpr ownership(ownership const&) = default;
  ownership(ownership&& right) noexcept
      : acquired_(right.consume()), frozen_(right.is_frozen()) {
  }
  ownership& operator=(ownership const&) = default;
  ownership& operator=(ownership&& right) noexcept {
    acquired_ = right.consume();
    frozen_ = right.is_frozen();
    return *this;
  }

  // Merges both ownerships together
  ownership operator|(ownership const& right) const noexcept {
    return ownership(is_acquired() && right.is_acquired(),
                     is_frozen() || right.is_frozen());
  }

  constexpr bool is_acquired() const noexcept {
    return acquired_;
  }
  constexpr bool is_frozen() const noexcept {
    return frozen_;
  }

  void release() noexcept {
    assert(is_acquired() && "Tried to release the ownership twice!");
    acquired_ = false;
  }
  void freeze(bool enabled = true) noexcept {
    assert(is_acquired() && "Tried to freeze a released object!");
    frozen_ = enabled;
  }

private:
  bool consume() noexcept {
    if (is_acquired()) {
      release();
      return true;
    }
    return false;
  }

  /// Is true when the object is in a valid state
  bool acquired_ : 1;
  /// Is true when the automatic invocation on destruction is disabled
  bool frozen_ : 1;
};

/// Hint for the compiler that this point should be unreachable
[[noreturn]] inline void unreachable() {
#if defined(_MSC_VER)
  __assume(false);
#elif defined(__GNUC__)
  __builtin_unreachable();
#elif defined(__has_builtin) && __has_builtin(__builtin_unreachable)
  __builtin_unreachable();
#endif
}

/// Causes the application to exit abnormally because we are
/// in an invalid state.
[[noreturn]] inline void trap() {
#if defined(_MSC_VER)
  __debugbreak();
#elif defined(__GNUC__)
  __builtin_trap();
#elif defined(__has_builtin) && __has_builtin(__builtin_trap)
  __builtin_trap();
#else
  *(volatile int*)0 = 0;
#endif
}

/// Exposes functionality to emulate later standard features
namespace emulation {
#ifndef CONTINUABLE_HAS_CXX17_FOLD_EXPRESSION
/// Combines the given arguments with the given folding function
template <typename F, typename First>
constexpr auto fold(F&& /*folder*/, First&& first) {
  return std::forward<First>(first);
}
/// Combines the given arguments with the given folding function
template <typename F, typename First, typename Second, typename... Rest>
auto fold(F&& folder, First&& first, Second&& second, Rest&&... rest) {
  auto res = folder(std::forward<First>(first), std::forward<Second>(second));
  return fold(std::forward<F>(folder), std::move(res),
              std::forward<Rest>(rest)...);
}
#endif // CONTINUABLE_HAS_CXX17_FOLD_EXPRESSION
} // namespace emulation
} // namespace util
} // namespace detail
} // namespace cti

#ifdef CONTINUABLE_CONSTEXPR_IF
#define CONTINUABLE_CONSTEXPR_IF(EXPR, TRUE_BRANCH, FALSE_BRANCH)
#else
#define CONTINUABLE_CONSTEXPR_IF(EXPR, TRUE_BRANCH, FALSE_BRANCH)
#endif // CONTINUABLE_CONSTEXPR_IF

#ifdef CONTINUABLE_HAS_CXX17_FOLD_EXPRESSION
#define CONTINUABLE_FOLD_EXPRESSION(OP, PACK) (... OP PACK)
#else
#define CONTINUABLE_FOLD_EXPRESSION(OP, PACK)                                  \
  cti::detail::util::emulation::fold(                                          \
      [](auto&& left, auto&& right) {                                          \
        return std::forward<decltype(left)>(left)                              \
            OP std::forward<decltype(right)>(right);                           \
      },                                                                       \
      PACK)
#endif // CONTINUABLE_HAS_CXX17_FOLD_EXPRESSION

#endif // CONTINUABLE_DETAIL_UTIL_HPP_INCLUDED_
