// Copyright (c) 2023 Louis-Charles Caron

// This file is part of the safe library (https://github.com/LouisCharlesC/safe).

// Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file or at https://opensource.org/licenses/MIT.

#pragma once

#include <cstddef>

namespace safe
{
namespace impl
{
// This set of template and specializations is used to extract the type of the last argument of a paramter pack.
template <typename... Ts> struct Last; // Base template, specializations cover all uses.
template <typename First, typename Second, typename... Others> struct Last<First, Second, Others...>
{
    using type = typename Last<Second, Others...>::type;
};
template <typename T> struct Last<T>
{
    using type = T;
};
template <> struct Last<>
{
    using type = void;
};

template<std::size_t... Is>
struct index_sequence {};
template<std::size_t N, std::size_t... Is>
struct index_sequence<N, Is...>
{
    using type = typename index_sequence<N-1, N-1, Is...>::type;
};
template<std::size_t... Is>
struct index_sequence<0, Is...>
{
    using type = index_sequence<Is...>;
};

template<std::size_t N>
constexpr typename index_sequence<N>::type make_index_sequence()
{
    return {};
}
} // namespace impl

template <typename... Ts> using Last = typename impl::Last<Ts...>::type;
} // namespace safe