/**
 * @file safetraits.h
 * @author L.-C. C.
 * @brief 
 * @version 0.1
 * @date 2019-01-20
 * 
 * @copyright Copyright (c) 2019
 * 
 */

namespace safe
{
	enum class AccessMode
	{
		ReadOnly,
		ReadWrite
	};

	template<template<typename> class LockType>
	struct LockTraits
	{
		static constexpr AccessMode DefaultAccessMode = AccessMode::ReadWrite;
	};
#if __cplusplus >= 201402L
	template<>
	struct LockTraits<std::shared_unique_lock>
	{
		static constexpr AccessMode DefaultAccessMode = AccessMode::ReadOnly;
	};
#endif // __cplusplus >= 201402L
} // namespace safe