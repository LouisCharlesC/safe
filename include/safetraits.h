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
	enum ReadOrWrite
	{
		ReadOnly,
		ReadWrite
	};

	template<template<typename> class LockType>
	struct LockTraits
	{
		static constexpr ReadOrWrite DefaultAccessMode = ReadWrite;
	};
#if __cplusplus >= 201402L
	template<>
	struct LockTraits<std::shared_lock>
	{
		static constexpr ReadOrWrite DefaultAccessMode = ReadOnly;
	};
#endif // __cplusplus >= 201402L
} // namespace safe