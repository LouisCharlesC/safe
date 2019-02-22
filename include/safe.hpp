/*
 * safe.hpp
 *
 *  Created on: Sep 21, 2018
 *      Author: lcc
 */

#pragma once

#include "safe.h"

#include "safetraits.h"

#include <utility>

namespace safe
{
	/**
	 * Lock
	 */
	template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
	template<template<typename> class LockType, AccessMode ReadOrWrite>
	template<typename... OtherLockArgs>
	Safe<ValueType, LockableType, DefaultReadOnlyLock>::Access<LockType, ReadOrWrite>::Access(const Safe<ValueType, LockableType, DefaultReadOnlyLock>& safe, OtherLockArgs&&... otherLockArgs):
		lock(safe.m_lockable.lockable, std::forward<OtherLockArgs>(otherLockArgs)...),
		m_value(safe.m_value)
	{
		static_assert(ReadOrWrite==AccessMode::ReadOnly, "AccessMode must be ReadOnly when you construct from a const Safe object.");
	}
	template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
	template<template<typename> class LockType, AccessMode ReadOrWrite>
	template<typename... OtherLockArgs>
	Safe<ValueType, LockableType, DefaultReadOnlyLock>::Access<LockType, ReadOrWrite>::Access(Safe<ValueType, LockableType, DefaultReadOnlyLock>& safe, OtherLockArgs&&... otherLockArgs):
		lock(safe.m_lockable.lockable, std::forward<OtherLockArgs>(otherLockArgs)...),
		m_value(safe.m_value)
	{}
	template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
	template<template<typename> class LockType, AccessMode ReadOrWrite>
	typename Safe<ValueType, LockableType, DefaultReadOnlyLock>::template Access<LockType, ReadOrWrite>::ConstPointerType Safe<ValueType, LockableType, DefaultReadOnlyLock>::Access<LockType, ReadOrWrite>::operator->() const noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
	template<template<typename> class LockType, AccessMode ReadOrWrite>
	typename Safe<ValueType, LockableType, DefaultReadOnlyLock>::template Access<LockType, ReadOrWrite>::PointerType Safe<ValueType, LockableType, DefaultReadOnlyLock>::Access<LockType, ReadOrWrite>::operator->() noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
	template<template<typename> class LockType, AccessMode ReadOrWrite>
	typename Safe<ValueType, LockableType, DefaultReadOnlyLock>::template Access<LockType, ReadOrWrite>::ConstReferenceType Safe<ValueType, LockableType, DefaultReadOnlyLock>::Access<LockType, ReadOrWrite>::operator*() const noexcept
	{
		return m_value;
	}
	template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
	template<template<typename> class LockType, AccessMode ReadOrWrite>
	typename Safe<ValueType, LockableType, DefaultReadOnlyLock>::template Access<LockType, ReadOrWrite>::ReferenceType Safe<ValueType, LockableType, DefaultReadOnlyLock>::Access<LockType, ReadOrWrite>::operator*() noexcept
	{
		return m_value;
	}

	/**
	 * Safe
	 */
  template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
  template<typename... ValueArgs>
  Safe<ValueType, LockableType, DefaultReadOnlyLock>::Safe(default_construct_lockable_t, ValueArgs&&... valueArgs):
		m_lockable(),
		m_value(std::forward<ValueArgs>(valueArgs)...)
  {}
  template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
  template<typename LockableArg, typename... ValueArgs>
  Safe<ValueType, LockableType, DefaultReadOnlyLock>::Safe(LockableArg&& lockableArg, ValueArgs&&... valueArgs):
		m_lockable(std::forward<LockableArg>(lockableArg)),
		m_value(std::forward<ValueArgs>(valueArgs)...)
  {}

  template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
	template<template<typename> class LockType, typename... OtherLockArgs>
	typename Safe<ValueType, LockableType, DefaultReadOnlyLock>::template Access<LockType, AccessMode::ReadOnly> Safe<ValueType, LockableType, DefaultReadOnlyLock>::access(OtherLockArgs&&... otherLockArgs) const
	{
		return {*this, std::forward<OtherLockArgs>(otherLockArgs)...};
	}
	template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
	template<template<typename> class LockType, typename... OtherLockArgs>
	typename Safe<ValueType, LockableType, DefaultReadOnlyLock>::template Access<LockType> Safe<ValueType, LockableType, DefaultReadOnlyLock>::access(OtherLockArgs&&... otherLockArgs)
	{
		return {*this, std::forward<OtherLockArgs>(otherLockArgs)...};
	}
	template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
	template<AccessMode ReadOrWrite, template<typename> class LockType, typename... OtherLockArgs>
	typename Safe<ValueType, LockableType, DefaultReadOnlyLock>::template Access<LockType, ReadOrWrite> Safe<ValueType, LockableType, DefaultReadOnlyLock>::access(OtherLockArgs&&... otherLockArgs)
	{
		return {*this, std::forward<OtherLockArgs>(otherLockArgs)...};
	}

  template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
  typename Safe<ValueType, LockableType, DefaultReadOnlyLock>::ConstReferenceType Safe<ValueType, LockableType, DefaultReadOnlyLock>::unsafe() const noexcept
	{
  	return m_value;
	}
  template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
  typename Safe<ValueType, LockableType, DefaultReadOnlyLock>::ReferenceType Safe<ValueType, LockableType, DefaultReadOnlyLock>::unsafe() noexcept
	{
  	return m_value;
	}
  template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
	const typename Safe<ValueType, LockableType, DefaultReadOnlyLock>::RemoveRefLockableType& Safe<ValueType, LockableType, DefaultReadOnlyLock>::lockable() const noexcept
  {
  	return m_lockable.lockable;
	}
  template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
  typename Safe<ValueType, LockableType, DefaultReadOnlyLock>::RemoveRefLockableType& Safe<ValueType, LockableType, DefaultReadOnlyLock>::lockable() noexcept
	{
  	return m_lockable.lockable;
	}
}  // namespace safe
