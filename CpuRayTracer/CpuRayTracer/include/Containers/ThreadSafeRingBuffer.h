#pragma once
#include <cstdint>
#include <mutex>

namespace CRT
{
	/// <summary>
	/// Thread safe implementation of a very simple ring buffer.
	/// </summary>
	template<typename T, size_t Capacity>
	class ThreadSafeRingBuffer
	{
	public:
		ThreadSafeRingBuffer();
		~ThreadSafeRingBuffer() = default;

		bool PushBack(const T& item);
		bool PopFront(T& item);

		size_t GetCapacity() const;

	private:
		size_t m_Head;
		size_t m_Tail;
		T m_Data[Capacity];

		std::mutex m_RingBufferMutex;
	};


	template<typename T, size_t Capacity>
	ThreadSafeRingBuffer<T, Capacity>::ThreadSafeRingBuffer() :
		m_Head(0),
		m_Tail(0)
	{}

	template<typename T, size_t Capacity>
	bool ThreadSafeRingBuffer<T, Capacity>::PopFront(T& item)
	{
		bool result = false;
		m_RingBufferMutex.lock();

		if (m_Tail != m_Head)
		{
			item = m_Data[m_Tail];
			m_Tail = (m_Tail + 1) % Capacity;
			result = true;
		}

		m_RingBufferMutex.unlock();
		return result;
	}

	template<typename T, size_t Capacity>
	bool ThreadSafeRingBuffer<T, Capacity>::PushBack(const T& item)
	{
		bool result = false;
		m_RingBufferMutex.lock();

		size_t next = (m_Head + 1) % Capacity;
		if (next != m_Tail)
		{
			m_Data[m_Head] = item;
			m_Head = next;
			result = true;
		}

		m_RingBufferMutex.unlock();
		return result;
	}

	template<typename T, size_t Capacity>
	size_t ThreadSafeRingBuffer<T, Capacity>::GetCapacity() const
	{
		return Capacity;
	}
}