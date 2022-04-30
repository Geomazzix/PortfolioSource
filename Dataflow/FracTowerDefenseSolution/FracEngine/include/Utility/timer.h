#pragma once
namespace Frac
{
	class Timer
	{
	public:
		/**
		 * \brief The timer imidiatly starts on construction
		 */
		Timer();
		/**
		 * \brief Get the Time since the last reset or construction
		 * \return time in seconds
		 */
		float GetTime();
		/**
		 * \brief reset the timer to the current time
		 */
		void Reset();
	private:
		std::chrono::high_resolution_clock::time_point begin;
	};
}
