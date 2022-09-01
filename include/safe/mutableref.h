// Copyright(c) 2019-2022 Louis-Charles Caron

// This file is part of the safe library.

// (https://github.com/LouisCharlesC/safe). Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file or at https://opensource.org/licenses/MIT.

#pragma once

#include <utility>

namespace safe
{
namespace impl
{
/**
 * @brief A helper class that defines a member variable of type Type. The variable is defined "mutable Type" if Type is
 * not a reference, the variable is "Type&" if Type is a reference.
 *
 * @tparam Type The type of the variable to define.
 */
template <typename Type> struct MutableIfNotReference
{
    /// Mutable Type object.
    mutable Type get;
};
/**
 * @brief Specialization of MutableIfNotReference for references.
 *
 * @tparam Type The type of the reference to define.
 */
template <typename Type> struct MutableIfNotReference<Type &>
{
    /// Reference to a Type object.
    Type &get;
};
} // namespace impl
} // namespace safe