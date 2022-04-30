#pragma once
#include <string>

class Version {
public:
	/**
	* Gets the current BuildVersion of the solution
	* @return: A string with BuildVersion as [date y/m/d].[most-recent changelist number]
	*/
	static std::string GetVersion();
#ifdef PLATFORM_WINDOWS
	/**
	* Stores the current BuildVersion of the solution in a BuildVersion.txt file
	*/
	static void StoreVersion();
#endif

private:
	Version() {};
	~Version() {};
};
