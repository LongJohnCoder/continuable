
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

#ifndef CONTINUABLE_DETAIL_TRAITS_HPP_INCLUDED_
#define CONTINUABLE_DETAIL_TRAITS_HPP_INCLUDED_

#include <cstdint>
#include <initializer_list>
#include <tuple>
#include <type_traits>
#include <utility>

#include <continuable/continuable-api.hpp>
#include <continuable/detail/features.hpp>

namespace cti {
namespace detail {
namespace traits {
/// \cond false
#define CTI__FOR_EACH_BOOLEAN_BIN_OP(CTI__OP__)                                \
  CTI__OP__(==)                                                                \
  CTI__OP__(!=) CTI__OP__(<=) CTI__OP__(>=) CTI__OP__(<) CTI__OP__(>)
#define CTI__FOR_EACH_BOOLEAN_UNA_OP(CTI__OP__) CTI__OP__(!)
#define CTI__FOR_EACH_INTEGRAL_BIN_OP(CTI__OP__)                               \
  CTI__OP__(*)                                                                 \
  CTI__OP__(/) CTI__OP__(+) CTI__OP__(-) CTI__FOR_EACH_BOOLEAN_BIN_OP(CTI__OP__)
#define CTI__FOR_EACH_INTEGRAL_UNA_OP(CTI__OP__)                               \
  CTI__OP__(~) CTI__FOR_EACH_BOOLEAN_UNA_OP(CTI__OP__)
/// \endcond

template <typename T, T Value>
struct constant : std::integral_constant<T, Value> {
/// \cond false
#define CTI__INST(CTI__OP)                                                     \
  template <typename OT, OT OValue>                                            \
  constexpr auto operator CTI__OP(std::integral_constant<OT, OValue>)          \
      const noexcept {                                                         \
    return constant<decltype((Value CTI__OP OValue)),                          \
                    (Value CTI__OP OValue)>{};                                 \
  }
  CTI__FOR_EACH_INTEGRAL_BIN_OP(CTI__INST)
#undef CTI__INST
#define CTI__INST(CTI__OP)                                                     \
  constexpr auto operator CTI__OP() const noexcept {                           \
    return constant<decltype((CTI__OP Value)), (CTI__OP Value)>{};             \
  }
  CTI__FOR_EACH_INTEGRAL_UNA_OP(CTI__INST)
#undef CTI__INST
  /// \endcond
};

template <bool Value>
struct constant<bool, Value> : std::integral_constant<bool, Value> {
/// \cond false
#define CTI__INST(CTI__OP)                                                     \
  template <typename OT, OT OValue>                                            \
  constexpr auto operator CTI__OP(std::integral_constant<bool, OValue>)        \
      const noexcept {                                                         \
    return constant<bool, (Value CTI__OP OValue)>{};                           \
  }
  CTI__FOR_EACH_BOOLEAN_BIN_OP(CTI__INST)
#undef CTI__INST
#define CTI__INST(CTI__OP)                                                     \
  constexpr auto operator CTI__OP() const noexcept {                           \
    return constant<bool, CTI__OP Value>{};                                    \
  }
  CTI__FOR_EACH_BOOLEAN_UNA_OP(CTI__INST)
#undef CTI__INST
  /// \endcond
};

template <bool Value>
using bool_constant = constant<bool, Value>;
template <std::size_t Value>
using size_constant = constant<std::size_t, Value>;

template <typename T, bool Value>
constexpr auto constant_of(std::integral_constant<T, Value> /*value*/ = {}) {
  return constant<T, Value>{};
}
template <std::size_t Value>
constexpr auto
size_constant_of(std::integral_constant<std::size_t, Value> /*value*/ = {}) {
  return size_constant<Value>{};
}
template <bool Value>
constexpr auto
bool_constant_of(std::integral_constant<bool, Value> /*value*/ = {}) {
  return bool_constant<Value>{};
}

#undef CTI__FOR_EACH_BOOLEAN_BIN_OP
#undef CTI__FOR_EACH_BOOLEAN_UNA_OP
#undef CTI__FOR_EACH_INTEGRAL_BIN_OP
#undef CTI__FOR_EACH_INTEGRAL_UNA_OP

/// Evaluates to the element at position I.
template <std::size_t I, typename... Args>
using at_t = decltype(std::get<I>(std::declval<std::tuple<Args...>>()));

/// Evaluates to an integral constant which represents the size
/// of the given pack.
template <typename... Args>
using size_of_t = size_constant<sizeof...(Args)>;

/// A tagging type for wrapping other types
template <typename... T>
struct identity {};
template <typename T>
struct identity<T> : std::common_type<T> {};

template <typename>
struct is_identity : std::false_type {};
template <typename... Args>
struct is_identity<identity<Args...>> : std::true_type {};

template <typename T>
constexpr identity<std::decay_t<T>> identity_of(T const& /*type*/) noexcept {
  return {};
}
template <typename... Args>
constexpr identity<Args...> identity_of(identity<Args...> /*type*/) noexcept {
  return {};
}
template <typename T>
using identify = std::conditional_t<is_identity<std::decay_t<T>>::value, T,
                                    identity<std::decay_t<T>>>;

template <std::size_t I, typename... T>
constexpr auto get(identity<T...>) noexcept {
  return identify<at_t<I, T...>>{};
}

namespace detail {
// Equivalent to C++17's std::void_t which targets a bug in GCC,
// that prevents correct SFINAE behavior.
// See http://stackoverflow.com/questions/35753920 for details.
template <typename...>
struct deduce_to_void : std::common_type<void> {};
} // namespace detail

/// C++17 like void_t type
template <typename... T>
using void_t = typename detail::deduce_to_void<T...>::type;

namespace detail {
template <typename T, typename Check, typename = void_t<>>
struct is_valid_impl : std::common_type<std::false_type> {};

template <typename T, typename Check>
struct is_valid_impl<T, Check,
                     void_t<decltype(std::declval<Check>()(std::declval<T>()))>>
    : std::common_type<std::true_type> {};

template <typename Type, typename TrueCallback>
constexpr void static_if_impl(std::true_type, Type&& type,
                              TrueCallback&& trueCallback) {
  std::forward<TrueCallback>(trueCallback)(std::forward<Type>(type));
}

template <typename Type, typename TrueCallback>
constexpr void static_if_impl(std::false_type, Type&& /*type*/,
                              TrueCallback&& /*trueCallback*/) {
}

template <typename Type, typename TrueCallback, typename FalseCallback>
constexpr auto static_if_impl(std::true_type, Type&& type,
                              TrueCallback&& trueCallback,
                              FalseCallback&& /*falseCallback*/) {
  return std::forward<TrueCallback>(trueCallback)(std::forward<Type>(type));
}

template <typename Type, typename TrueCallback, typename FalseCallback>
constexpr auto static_if_impl(std::false_type, Type&& type,
                              TrueCallback&& /*trueCallback*/,
                              FalseCallback&& falseCallback) {
  return std::forward<FalseCallback>(falseCallback)(std::forward<Type>(type));
}

/// Applies the given callable to all objects in a sequence
template <typename C>
struct apply_to_all {
  C callable;

  template <typename... T>
  constexpr void operator()(T&&... args) {
    (void)std::initializer_list<int>{
        0, ((void)callable(std::forward<decltype(args)>(args)), 0)...};
  }
};
} // namespace detail

/// Returns the pack size of the given type
template <typename... Args>
constexpr auto pack_size_of(identity<std::tuple<Args...>>) noexcept {
  return size_of_t<Args...>{};
}
/// Returns the pack size of the given type
template <typename First, typename Second>
constexpr auto pack_size_of(identity<std::pair<First, Second>>) noexcept {
  return size_of_t<First, Second>{};
}
/// Returns the pack size of the given type
template <typename... Args>
constexpr auto pack_size_of(identity<Args...>) noexcept {
  return size_of_t<Args...>{};
}

/// Returns an index sequence of the given type
template <typename T>
constexpr auto sequence_of(T&& /*sequenceable*/) noexcept {
  return std::make_index_sequence<decltype(
      pack_size_of(std::declval<T>()))::value>();
}

/// Returns a check which returns a true type if the current value
/// is below the
template <std::size_t End>
constexpr auto is_less_than(size_constant<End> end) noexcept {
  return [=](auto current) { return end > current; };
}

/// Compile-time check for validating a certain expression
template <typename T, typename Check>
constexpr auto is_valid(T&& /*type*/, Check&& /*check*/) noexcept {
  return typename detail::is_valid_impl<T, Check>::type{};
}

/// Creates a static functional validator object.
template <typename Check>
constexpr auto validator_of(Check&& check) noexcept(
    std::is_nothrow_move_constructible<std::decay_t<Check>>::value) {
  return [check = std::forward<Check>(check)](auto&& matchable) {
    return is_valid(std::forward<decltype(matchable)>(matchable), check);
  };
}

/// Invokes the callback only if the given type matches the check
template <typename Type, typename Check, typename TrueCallback>
constexpr void static_if(Type&& type, Check&& check,
                         TrueCallback&& trueCallback) {
  detail::static_if_impl(std::forward<Check>(check)(type),
                         std::forward<Type>(type),
                         std::forward<TrueCallback>(trueCallback));
}

/// Invokes the callback only if the given type matches the check
template <typename Type, typename Check, typename TrueCallback,
          typename FalseCallback>
constexpr auto static_if(Type&& type, Check&& check,
                         TrueCallback&& trueCallback,
                         FalseCallback&& falseCallback) {
  return detail::static_if_impl(std::forward<Check>(check)(type),
                                std::forward<Type>(type),
                                std::forward<TrueCallback>(trueCallback),
                                std::forward<FalseCallback>(falseCallback));
}

/// A compile-time while loop, which loops as long the value matches
/// the predicate. The handler shall return the next value.
template <typename Value, typename Predicate, typename Handler>
constexpr auto static_while(Value&& value, Predicate&& predicate,
                            Handler&& handler) {
  return static_if(std::forward<Value>(value), predicate,
                   [&](auto&& result) mutable {
                     return static_while(
                         handler(std::forward<decltype(result)>(result)),
                         std::forward<Predicate>(predicate),
                         std::forward<Handler>(handler));
                   },
                   [&](auto&& result) mutable {
                     return std::forward<decltype(result)>(result);
                   });
}

/// Returns a validator which checks whether the given sequenceable is empty
inline auto is_empty() noexcept {
  return [](auto const& checkable) {
    return pack_size_of(checkable) == size_constant_of<0>();
  };
}

/// Calls the given unpacker with the content of the given sequence
template <typename U, std::size_t... I>
constexpr auto unpack(std::integer_sequence<std::size_t, I...>, U&& unpacker) {
  return std::forward<U>(unpacker)(size_constant_of<I>()...);
}

/// Calls the given unpacker with the content of the given sequenceable
template <typename F, typename U, std::size_t... I>
constexpr auto unpack(F&& firstSequenceable, U&& unpacker,
                      std::integer_sequence<std::size_t, I...>) {
  using std::get;
  (void)firstSequenceable;
  return std::forward<U>(unpacker)(
      get<I>(std::forward<F>(firstSequenceable))...);
}
/// Calls the given unpacker with the content of the given sequenceable
template <typename F, typename S, typename U, std::size_t... IF,
          std::size_t... IS>
constexpr auto unpack(F&& firstSequenceable, S&& secondSequenceable,
                      U&& unpacker, std::integer_sequence<std::size_t, IF...>,
                      std::integer_sequence<std::size_t, IS...>) {
  using std::get;
  (void)firstSequenceable;
  (void)secondSequenceable;
  return std::forward<U>(unpacker)(
      get<IF>(std::forward<F>(firstSequenceable))...,
      get<IS>(std::forward<S>(secondSequenceable))...);
}
/// Calls the given unpacker with the content of the given sequenceable
template <typename F, typename U>
constexpr auto unpack(F&& firstSequenceable, U&& unpacker) {
  return unpack(std::forward<F>(firstSequenceable), std::forward<U>(unpacker),
                sequence_of(identify<decltype(firstSequenceable)>{}));
}
/// Calls the given unpacker with the content of the given sequenceables
template <typename F, typename S, typename U>
constexpr auto unpack(F&& firstSequenceable, S&& secondSequenceable,
                      U&& unpacker) {
  return unpack(std::forward<F>(firstSequenceable),
                std::forward<S>(secondSequenceable), std::forward<U>(unpacker),
                sequence_of(identity_of(firstSequenceable)),
                sequence_of(identity_of(secondSequenceable)));
}

/// Applies the handler function to each element contained in the sequenceable
// TODO Maybe crashes MSVC in constexpr mode
template <typename Sequenceable, typename Handler>
constexpr void static_for_each_in(Sequenceable&& sequenceable,
                                  Handler&& handler) {
  unpack(std::forward<Sequenceable>(sequenceable),
         detail::apply_to_all<std::decay_t<Handler>>{
             std::forward<Handler>(handler)});
}

/// Adds the given type at the back of the left sequenceable
template <typename Left, typename Element>
constexpr auto push(Left&& left, Element&& element) {
  return unpack(std::forward<Left>(left), [&](auto&&... args) {
    return std::make_tuple(std::forward<decltype(args)>(args)...,
                           std::forward<Element>(element));
  });
}

/// Adds the element to the back of the identity
template <typename... Args, typename Element>
constexpr auto push(identity<Args...>, identity<Element>) noexcept {
  return identity<Args..., Element>{};
}

/// Removes the first element from the identity
template <typename First, typename... Rest>
constexpr auto pop_first(identity<First, Rest...>) noexcept {
  return identity<Rest...>{};
}

/// Returns the merged sequence
template <typename Left>
constexpr auto merge(Left&& left) {
  return std::forward<Left>(left);
}
/// Merges the left sequenceable with the right ones
template <typename Left, typename Right, typename... Rest>
constexpr auto merge(Left&& left, Right&& right, Rest&&... rest) {
  // Merge the left with the right sequenceable and
  // merge the result with the rest.
  return merge(unpack(std::forward<Left>(left), std::forward<Right>(right),
                      [&](auto&&... args) {
                        // Maybe use: template <template<typename...> class T,
                        // typename... Args>
                        return std::make_tuple(
                            std::forward<decltype(args)>(args)...);
                      }),
               std::forward<Rest>(rest)...);
}
/// Merges the left identity with the right ones
template <typename... LeftArgs, typename... RightArgs, typename... Rest>
constexpr auto merge(identity<LeftArgs...> /*left*/,
                     identity<RightArgs...> /*right*/, Rest&&... rest) {
  return merge(identity<LeftArgs..., RightArgs...>{},
               std::forward<Rest>(rest)...);
}

/// Deduces to a std::false_type
template <typename T>
using fail = std::integral_constant<bool, !std::is_same<T, T>::value>;
} // namespace traits
} // namespace detail
} // namespace cti

#endif // CONTINUABLE_DETAIL_TRAITS_HPP_INCLUDED_
