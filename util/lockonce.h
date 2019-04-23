/**
 * @file lockonce.h
 * @author L.-C. C.
 * @brief 
 * @version 0.1
 * @date 2019-01-18
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#pragma once

#include <mutex>

namespace safe
{
	template<typename LockableType>
	class LockOnce: private std::unique_lock<LockableType>
	{
	public:
		using std::unique_lock<LockableType>::unique_lock;

		using std::unique_lock<LockableType>::lock;
		using std::unique_lock<LockableType>::try_lock;
		using std::unique_lock<LockableType>::try_lock_for;
		using std::unique_lock<LockableType>::try_lock_until;

		using std::unique_lock<LockableType>::operator=;
		using std::unique_lock<LockableType>::swap;
		using std::unique_lock<LockableType>::release;

		using std::unique_lock<LockableType>::owns_lock;
		using std::unique_lock<LockableType>::operator bool;
		const LockableType* mutex() const noexcept {return std::unique_lock<LockableType>::mutex();}
	};
} // namespace safe