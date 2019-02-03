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
	template<template<typename> class LockType, ReadOrWrite AccessMode>
	Safe<ValueType, LockableType>::Access<LockType, AccessMode>::Access(const Safe<ValueType, LockableType>& safe) noexcept:
		lock(safe.m_lockable.lockable),
		m_value(safe.m_value)
	{
		static_assert(AccessMode==ReadOnly, "AccessMode must be ReadOnly when you construct from a const Safe object.");
	}
	template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, ReadOrWrite AccessMode>
	Safe<ValueType, LockableType>::Access<LockType, AccessMode>::Access(Safe<ValueType, LockableType>& safe) noexcept:
		lock(safe.m_lockable.lockable),
		m_value(safe.m_value)
	{}
	template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, ReadOrWrite AccessMode>
	template<typename... LockArgs>
	Safe<ValueType, LockableType>::Access<LockType, AccessMode>::Access(ReferenceType value, LockArgs&&... lockArgs):
		lock(std::forward<LockArgs>(lockArgs)...),
		m_value(value)
	{}
	template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, ReadOrWrite AccessMode>
	typename Safe<ValueType, LockableType>::template Access<LockType, AccessMode>::ConstPointerType Safe<ValueType, LockableType>::Access<LockType, AccessMode>::operator->() const noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, ReadOrWrite AccessMode>
	typename Safe<ValueType, LockableType>::template Access<LockType, AccessMode>::PointerType Safe<ValueType, LockableType>::Access<LockType, AccessMode>::operator->() noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, ReadOrWrite AccessMode>
	typename Safe<ValueType, LockableType>::template Access<LockType, AccessMode>::ConstReferenceType Safe<ValueType, LockableType>::Access<LockType, AccessMode>::operator*() const noexcept
	{
		return m_value;
	}
	template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, ReadOrWrite AccessMode>
	typename Safe<ValueType, LockableType>::template Access<LockType, AccessMode>::ReferenceType Safe<ValueType, LockableType>::Access<LockType, AccessMode>::operator*() noexcept
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
	template<template<typename> class LockType>
	typename Safe<ValueType, LockableType>::template Access<LockType, ReadOnly> Safe<ValueType, LockableType>::access() const
	{
		return {*this};
	}

  template<typename ValueType, typename LockableType>
	template<template<typename> class LockType>
	typename Safe<ValueType, LockableType>::template Access<LockType, LockTraits<LockType>::DefaultAccessMode> Safe<ValueType, LockableType>::access()// -> typename std::conditional<LockTraits<LockType>::isShared, Access<LockType, ReadOnly>, Access<LockType, ReadWrite>>::type
	{
		return {*this};
	}

  template<typename ValueType, typename LockableType>
	template<template<typename> class LockType, ReadOrWrite AccessMode>
	typename Safe<ValueType, LockableType>::template Access<LockType, AccessMode> Safe<ValueType, LockableType>::access()
	{
		return {*this};
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
