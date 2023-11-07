// Copyright (c) 2023 Louis-Charles Caron

// This file is part of the safe library (https://github.com/LouisCharlesC/safe).

// Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file or at https://opensource.org/licenses/MIT.

#pragma once

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
} // namespace impl

template <typename... Ts> using Last = typename impl::Last<Ts...>::type;
} // namespace safe