#include <pch.h>
#include "Utility/Timer.h"


/*
 * According to the Switch SDK the std::chrono functions should be supported 
 */
Frac::Timer::Timer()
{
	begin = std::chrono::high_resolution_clock::now();
}


float Frac::Timer::GetTime()
{
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
	return static_cast<float>(time_span.count());
}

void Frac::Timer::Reset()
{
	begin = std::chrono::high_resolution_clock::now();
}
