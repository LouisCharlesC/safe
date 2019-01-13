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
	template<typename LockType, bool Shared>
	Safe<ValueType, LockableType>::AccessImpl<LockType, Shared>::AccessImpl(const Safe<ValueType, LockableType>& safe) noexcept:
		lock(safe.m_lockable.lockable),
		m_value(safe.m_value)
	{}
	template<typename ValueType, typename LockableType>
	template<typename LockType, bool Shared>
	Safe<ValueType, LockableType>::AccessImpl<LockType, Shared>::AccessImpl(Safe<ValueType, LockableType>& safe) noexcept:
		lock(safe.m_lockable.lockable),
		m_value(safe.m_value)
	{}
	template<typename ValueType, typename LockableType>
	template<typename LockType, bool Shared>
	template<typename... LockArgs>
	Safe<ValueType, LockableType>::AccessImpl<LockType, Shared>::AccessImpl(ReferenceType value, LockArgs&&... lockArgs):
		lock(std::forward<LockArgs>(lockArgs)...),
		m_value(value)
	{}
	template<typename ValueType, typename LockableType>
	template<typename LockType, bool Shared>
	typename Safe<ValueType, LockableType>::template AccessImpl<LockType, Shared>::ConstPointerType Safe<ValueType, LockableType>::AccessImpl<LockType, Shared>::operator->() const noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType>
	template<typename LockType, bool Shared>
	typename Safe<ValueType, LockableType>::template AccessImpl<LockType, Shared>::PointerType Safe<ValueType, LockableType>::AccessImpl<LockType, Shared>::operator->() noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType>
	template<typename LockType, bool Shared>
	typename Safe<ValueType, LockableType>::template AccessImpl<LockType, Shared>::ConstReferenceType Safe<ValueType, LockableType>::AccessImpl<LockType, Shared>::operator*() const noexcept
	{
		return m_value;
	}
	template<typename ValueType, typename LockableType>
	template<typename LockType, bool Shared>
	typename Safe<ValueType, LockableType>::template AccessImpl<LockType, Shared>::ReferenceType Safe<ValueType, LockableType>::AccessImpl<LockType, Shared>::operator*() noexcept
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
