#pragma once
#include <cstdint>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "Containers/ThreadSafeRingBuffer.h"

namespace CRT
{
	using ThreadJob = std::function<void()>;

	/// <summary>
	/// The job system manages multiple rays at the same time in order to improve the render time.
	/// https://wickedengine.net/2018/11/24/simple-job-system-using-standard-c/
	/// </summary>
	class JobSystem
	{
	public:
		JobSystem();
		~JobSystem() = default;

		void Initialize();
		void Execute(const ThreadJob& job);

		bool IsBusy();
		void Wait();

	private:
		ThreadSafeRingBuffer<ThreadJob, 256> m_JobPool;
		uint64_t m_CurrentLabel;
		std::atomic_uint64_t m_FinishedValue;
		std::mutex m_LockMutex;
		std::condition_variable m_WakeCondition;

		void Poll();
		void WorkerThread();
	};
}