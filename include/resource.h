/*
 * localressources.h
 *
 *  Created on: Oct 11, 2018
 *      Author: lcc
 */

#ifndef RESOURCE_H_
#define RESOURCE_H_

#include "pushlist.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace safe {
	template<typename ResourceType>
	class Resource
	{
//	using ResourceType = int;

	struct SetFlag {};
	struct ClearFlag {};
  struct FlagAndResource
	{
  	std::atomic_flag flag;
  	ResourceType resource;
  	template<typename... Args>
  	FlagAndResource(SetFlag, Args&&... args): resource(std::forward<Args>(args)...) {flag.clear();}
  	template<typename... Args>
  	FlagAndResource(ClearFlag, Args&&... args): resource(std::forward<Args>(args)...) {flag.test_and_set();}
	};

	public:
		class Handle
		{
		public:
			Handle(FlagAndResource& m_flagAndResource):
				m_flagAndResource(&m_flagAndResource)
			{}
			Handle(const Handle&) = delete;
			Handle(Handle&& other):
				m_flagAndResource(other.m_flagAndResource)
			{
				other.m_flagAndResource = nullptr;
			}
			Handle& operator =(const Handle&) = delete;
			Handle& operator =(Handle&& other)
			{
				if (m_flagAndResource)
				{
					m_flagAndResource->flag.clear();
				}
				m_flagAndResource = other.m_flagAndResource;
				other.m_flagAndResource = nullptr;
				return *this;
			}
			~Handle()
			{
				if (m_flagAndResource)
				{
					m_flagAndResource->flag.clear();
				}
			}

	    const ResourceType* operator->() const noexcept
			{
				return &m_flagAndResource->resource;
			}
	    ResourceType* operator->() noexcept
			{
				return &m_flagAndResource->resource;
			}
	    const ResourceType& operator*() const noexcept
			{
				return m_flagAndResource->resource;
			}
	    ResourceType& operator*() noexcept
			{
				return m_flagAndResource->resource;
			}

		private:
			FlagAndResource* m_flagAndResource;
		};

		template<typename... Args>
    Handle get(Args&&... args)
    {
			auto entry = std::find_if(m_flagAndResources.begin(), m_flagAndResources.end(), [](FlagAndResource& flagAndResource){return !flagAndResource.flag.test_and_set();});
      if (entry == m_flagAndResources.endSentry())
      {
      	entry = m_flagAndResources.push(SetFlag(), std::forward<Args>(args)...);
      }
      return {*entry};
    }

		template<typename... Args>
		void push(Args&&... args)
		{
			m_flagAndResources.push(ClearFlag(), std::forward<Args>(args)...);
		}

		template<typename... Args>
		void push_n(std::size_t n, const ResourceType& resource)
		{
			for (; n != 0; --n)
			{
				m_flagAndResources.push(ClearFlag(), resource);
			}
		}

	private:
		PushList<FlagAndResource> m_flagAndResources;
	};

}  // namespace safe

#endif /* RESOURCE_H_ */
