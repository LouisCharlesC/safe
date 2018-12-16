/*
 * lffl.h
 *
 *  Created on: Nov. 15, 2018
 *      Author: lcc
 */

#ifndef PUSH_LIST_H_
#define PUSH_LIST_H_

#include <atomic>
#include <iterator>
#include <memory>
#include <utility>

namespace safe {
	template<typename ValueType>
	class PushList
	{
		class Node {
		public:
			template<typename... Args>
			Node(Node* next, Args&&... args):
				value(std::forward<Args>(args)...),
				next(next)
			{}

			ValueType value;
			std::unique_ptr<Node> next;
		};

	public:
		class Iterator;
		class EndSentry;
		class ConstIterator {
		public:
	    using iterator_category = std::forward_iterator_tag;
	    using value_type = ValueType;
	    using difference_type = std::ptrdiff_t;
	    using pointer = const ValueType*;
	    using reference = const ValueType&;

	    ConstIterator():
	    	m_node(nullptr)
	    {}
	    ConstIterator(Node* node):
	    	m_node(node)
	    {}
	    ConstIterator(const Iterator& other):
	    	m_node(other.m_node)
	    {}
	    ConstIterator(EndSentry):
	    	m_node(nullptr)
	    {}

	    reference operator*() const noexcept
	    {
	    	return m_node->value;
	    }

	    pointer operator->() const noexcept
	    {
	    	return &m_node->value;
	    }

	    ConstIterator& operator++() noexcept
	    {
	      m_node = m_node->next.get();
	      return *this;
	    }

	    ConstIterator operator++(int) noexcept
	    {
	      const auto ret = Iterator(m_node);
	      m_node = m_node->next.get();
	      return ret;
	    }

			bool operator ==(const ConstIterator other)
			{
				return m_node == other.m_node;
			}
			bool operator ==(const EndSentry)
			{
				return m_node == nullptr;
			}
			bool operator !=(const ConstIterator other)
			{
				return m_node != other.m_node;
			}
			bool operator !=(const EndSentry)
			{
				return m_node != nullptr;
			}

		private:
	    Node* m_node;
		};

		class Iterator {
			friend ConstIterator;
		public:
	    using iterator_category = std::forward_iterator_tag;
	    using value_type = ValueType;
	    using difference_type = std::ptrdiff_t;
	    using pointer = ValueType*;
	    using reference = ValueType&;

	    Iterator():
	    	m_node(nullptr)
	    {}
	    Iterator(Node* node):
	    	m_node(node)
	    {}
	    Iterator(EndSentry):
	    	m_node(nullptr)
	    {}

	    reference operator*() const noexcept
	    {
	    	return m_node->value;
	    }

	    pointer operator->() const noexcept
	    {
	    	return &m_node->value;
	    }

	    Iterator& operator++() noexcept
	    {
	      m_node = m_node->next.get();
	      return *this;
	    }

	    Iterator operator++(int) noexcept
	    {
	      const auto ret = Iterator(m_node);
	      m_node = m_node->next.get();
	      return ret;
	    }

			bool operator ==(const Iterator other)
			{
				return m_node == other.m_node;
			}
			bool operator ==(const EndSentry)
			{
				return m_node == nullptr;
			}
			bool operator !=(const Iterator other)
			{
				return m_node != other.m_node;
			}
			bool operator !=(const EndSentry)
			{
				return m_node != nullptr;
			}

		private:
	    Node* m_node;
		};

		class EndSentry
		{
		public:
	    using iterator_category = std::forward_iterator_tag;
	    using value_type = ValueType;
	    using difference_type = std::ptrdiff_t;
	    using pointer = ValueType*;
	    using reference = ValueType&;

			bool operator ==(const ConstIterator other)
			{
				return other.m_node == nullptr;
			}
			bool operator !=(const ConstIterator other)
			{
				return other.m_node != nullptr;
			}
			bool operator ==(const Iterator other)
			{
				return other.m_node == nullptr;
			}
			bool operator !=(const Iterator other)
			{
				return other.m_node != nullptr;
			}
		};

		PushList():
			m_begin(nullptr)
		{}

		~PushList()
		{
			delete m_begin.load();
		}

		template<typename... Args>
		Iterator push(Args&&... args)
		{
			Node* ptr = m_begin;
			Node* const node = new Node(ptr, std::forward<Args>(args)...);
			while (!m_begin.compare_exchange_weak(ptr, node))
			{
				node->next.release();
				node->next.reset(ptr);
			}
			return {node};
		}

		bool empty() const
		{
			return m_begin == nullptr;
		}

		ConstIterator cbegin() const
		{
			return {m_begin.load()};
		}
		ConstIterator begin() const
		{
			return {m_begin.load()};
		}
		Iterator begin()
		{
			return {m_begin.load()};
		}
		ConstIterator cend() const
		{
			return {endSentry()};
		}
		ConstIterator end() const
		{
			return {endSentry()};
		}
		Iterator end()
		{
			return {endSentry()};
		}
		EndSentry endSentry() const
		{
			return {};
		}

	private:
		std::atomic<Node*> m_begin;
	};
}  // namespace safe

#endif /* PUSH_LIST_H_ */
