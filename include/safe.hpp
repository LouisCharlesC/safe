/*
 * safe.hpp
 *
 *  Created on: Sep 21, 2018
 *      Author: lcc
 */

#ifndef SAFE_HPP_
#define SAFE_HPP_

#include "safe.h"

namespace safe
{
	/**
	 * Lock
	 */
	template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, AccessMode ReadOrWrite>
	template<typename... OtherLockArgs>
	Safe<ValueType, LockableType>::Access<LockType, ReadOrWrite>::Access(const Safe<ValueType, LockableType>& safe, OtherLockArgs&&... otherLockArgs) noexcept(noexcept(LockType<RemoveRefLockableType>(safe.m_lockable.lockable, std::forward<OtherLockArgs>(otherLockArgs)...))):
		lock(safe.m_lockable.lockable, std::forward<OtherLockArgs>(otherLockArgs)...),
		m_value(safe.m_value)
	{
		static_assert(ReadOrWrite==AccessMode::ReadOnly, "ReadOrWrite must be ReadOnly when you construct from a const Safe object.");
	}
	template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, AccessMode ReadOrWrite>
	template<typename... OtherLockArgs>
	Safe<ValueType, LockableType>::Access<LockType, ReadOrWrite>::Access(Safe<ValueType, LockableType>& safe, OtherLockArgs&&... otherLockArgs) noexcept(noexcept(LockType<RemoveRefLockableType>(safe.m_lockable.lockable, std::forward<OtherLockArgs>(otherLockArgs)...))):
		lock(safe.m_lockable.lockable, std::forward<OtherLockArgs>(otherLockArgs)...),
		m_value(safe.m_value)
	{}
	template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, AccessMode ReadOrWrite>
	typename Safe<ValueType, LockableType>::template Access<LockType, ReadOrWrite>::ConstPointerType Safe<ValueType, LockableType>::Access<LockType, ReadOrWrite>::operator->() const noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, AccessMode ReadOrWrite>
	typename Safe<ValueType, LockableType>::template Access<LockType, ReadOrWrite>::PointerType Safe<ValueType, LockableType>::Access<LockType, ReadOrWrite>::operator->() noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, AccessMode ReadOrWrite>
	typename Safe<ValueType, LockableType>::template Access<LockType, ReadOrWrite>::ConstReferenceType Safe<ValueType, LockableType>::Access<LockType, ReadOrWrite>::operator*() const noexcept
	{
		return m_value;
	}
	template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, AccessMode ReadOrWrite>
	typename Safe<ValueType, LockableType>::template Access<LockType, ReadOrWrite>::ReferenceType Safe<ValueType, LockableType>::Access<LockType, ReadOrWrite>::operator*() noexcept
	{
		return m_value;
	}

	/**
	 * Safe
	 */
  template<typename ValueType, typename LockableType>
  template<typename... ValueArgs>
  Safe<ValueType, LockableType>::Safe(default_construct_lockable_t, ValueArgs&&... valueArgs):
		m_lockable(),
		m_value(std::forward<ValueArgs>(valueArgs)...)
  {}
  template<typename ValueType, typename LockableType>
  template<typename LockableArg, typename... ValueArgs>
  Safe<ValueType, LockableType>::Safe(LockableArg&& lockableArg, ValueArgs&&... valueArgs):
		m_lockable(std::forward<LockableArg>(lockableArg)),
		m_value(std::forward<ValueArgs>(valueArgs)...)
  {}

  template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, typename... OtherLockArgs>
	typename Safe<ValueType, LockableType>::template Access<LockType, AccessMode::ReadOnly> Safe<ValueType, LockableType>::access(OtherLockArgs&&... otherLockArgs) const
	{
		return {*this, std::forward<OtherLockArgs>(otherLockArgs)...};
	}

  template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, typename... OtherLockArgs>
	typename Safe<ValueType, LockableType>::template Access<LockType, LockTraits<LockType>::DefaultAccessMode> Safe<ValueType, LockableType>::access(OtherLockArgs&&... otherLockArgs)
	{
		return {*this, std::forward<OtherLockArgs>(otherLockArgs)...};
	}

  template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, AccessMode ReadOrWrite, typename... OtherLockArgs>
	typename Safe<ValueType, LockableType>::template Access<LockType, ReadOrWrite> Safe<ValueType, LockableType>::access(OtherLockArgs&&... otherLockArgs)
	{
		return {*this, std::forward<OtherLockArgs>(otherLockArgs)...};
	}

  template<typename ValueType, typename LockableType>
  typename Safe<ValueType, LockableType>::ConstReferenceType Safe<ValueType, LockableType>::unsafe() const noexcept
	{
  	return m_value;
	}
  template<typename ValueType, typename LockableType>
  typename Safe<ValueType, LockableType>::ReferenceType Safe<ValueType, LockableType>::unsafe() noexcept
	{
  	return m_value;
	}
  template<typename ValueType, typename LockableType>
	const typename Safe<ValueType, LockableType>::RemoveRefLockableType& Safe<ValueType, LockableType>::lockable() const noexcept
  // typename ConstReferenceType Safe<ValueType, LockableType>::unsafe() const noexcept
	{
  	return m_lockable.lockable;
	}
  template<typename ValueType, typename LockableType>
  typename Safe<ValueType, LockableType>::RemoveRefLockableType& Safe<ValueType, LockableType>::lockable() noexcept
	{
  	return m_lockable.lockable;
	}
}  // namespace safe

#endif /* SAFE_HPP_ */
