#include "TDpch.h"
#include "Tools/Version.h"
#include <fstream>
#include <filesystem>

#define STRINGIFY(x) #x

const char* changelistString =
#include "../../Version.txt"
	;

const char changelistVersion[6]
{
	changelistString[7],
	changelistString[8],
	changelistString[9],
	changelistString[10],
	changelistString[11],
	changelistString[12]
};

std::string Version::GetVersion()
{
	unsigned int month = 0;
	unsigned int day = 0;
	unsigned int year = 0;

	const char* months[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	char temp[] = __DATE__;

	year = atoi(temp + 9);
	*(temp + 6) = 0;
	day = atoi(temp + 4);
	*(temp + 3) = 0;
	for (int i = 0; i < 12; i++)
	{
		if (!strcmp(temp, months[i]))
		{
			month = i + 1;
			break;
		}
	}

	std::string dayString = day < 10 ? "0" + std::to_string(day) : std::to_string(day);
	std::string monthString = month < 10 ? "0" + std::to_string(month) : std::to_string(month);
	std::string yearString = year < 10 ? "0" + std::to_string(year) : std::to_string(year);

	std::string version(changelistVersion);
	version = version.substr(0, 6);
	version = yearString + monthString + dayString + "." + version;
	return version;
}
#ifdef PLATFORM_WINDOWS
void Version::StoreVersion()
{
	std::ofstream buildVersion = std::ofstream();

	std::string fileDirectory = std::filesystem::current_path().string() + "\\";
	std::string filePath = fileDirectory + "BuildVersion.txt";

	buildVersion.open(filePath, std::ios::out);

	buildVersion << GetVersion();

	buildVersion.close();
}
#endif