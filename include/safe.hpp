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
	template<typename LockType, ReadOrWrite AccessType>
	Safe<ValueType, LockableType>::AccessImpl<LockType, AccessType>::AccessImpl(const Safe<ValueType, LockableType>& safe) noexcept:
		lock(safe.m_lockable.lockable),
		m_value(safe.m_value)
	{
		static_assert(AccessType==ReadOnly, "Can only construct ReadOnly Access from const Safe.");
	}
	template<typename ValueType, typename LockableType>
	template<typename LockType, ReadOrWrite AccessType>
	Safe<ValueType, LockableType>::AccessImpl<LockType, AccessType>::AccessImpl(Safe<ValueType, LockableType>& safe) noexcept:
		lock(safe.m_lockable.lockable),
		m_value(safe.m_value)
	{}
	template<typename ValueType, typename LockableType>
	template<typename LockType, ReadOrWrite AccessType>
	template<typename... LockArgs>
	Safe<ValueType, LockableType>::AccessImpl<LockType, AccessType>::AccessImpl(ReferenceType value, LockArgs&&... lockArgs):
		lock(std::forward<LockArgs>(lockArgs)...),
		m_value(value)
	{}
	template<typename ValueType, typename LockableType>
	template<typename LockType, ReadOrWrite AccessType>
	typename Safe<ValueType, LockableType>::template AccessImpl<LockType, AccessType>::ConstPointerType Safe<ValueType, LockableType>::AccessImpl<LockType, AccessType>::operator->() const noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType>
	template<typename LockType, ReadOrWrite AccessType>
	typename Safe<ValueType, LockableType>::template AccessImpl<LockType, AccessType>::PointerType Safe<ValueType, LockableType>::AccessImpl<LockType, AccessType>::operator->() noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType>
	template<typename LockType, ReadOrWrite AccessType>
	typename Safe<ValueType, LockableType>::template AccessImpl<LockType, AccessType>::ConstReferenceType Safe<ValueType, LockableType>::AccessImpl<LockType, AccessType>::operator*() const noexcept
	{
		return m_value;
	}
	template<typename ValueType, typename LockableType>
	template<typename LockType, ReadOrWrite AccessType>
	typename Safe<ValueType, LockableType>::template AccessImpl<LockType, AccessType>::ReferenceType Safe<ValueType, LockableType>::AccessImpl<LockType, AccessType>::operator*() noexcept
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
	template<template<typename> class LockType, ReadOrWrite AccessType>
	typename Safe<ValueType, LockableType>::template Access<LockType, AccessType> Safe<ValueType, LockableType>::access()
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
