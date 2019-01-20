/**
 * @file accesstraits.h
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
	template<template<typename> class LockType>
	struct LockTraits
	{
		static constexpr bool isShared = false;
	};
} // namespace safe