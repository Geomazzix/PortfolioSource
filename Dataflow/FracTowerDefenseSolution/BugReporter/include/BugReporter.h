#pragma once
#include <string>
#include <filesystem>
#include <Windows.h>
#include <shellapi.h>
namespace BugReporter 
{
	class BugReporter
	{
	public:
		/**
		* Opens the BugReporter application
		*/
		static void OpenApplication()
		{
			LPCWSTR lpOperation = L"open";
			LPCWSTR lpFile;

			// Launching the application from within VS returns different filepaths (instead of launching the application from bin it launches it from TDGame)
			std::string currentFilePath = std::filesystem::current_path().string();
			if (currentFilePath.find("TDGame") != std::string::npos)
			{
#if defined (CONFIG_DEBUG)
				lpFile = L"\"..\\bin\\Debug\\x64\\BugReporter.exe\"";
#elif defined (CONFIG_DEVELOP)
				lpFile = L"\"..\\bin\\Develop\\x64\\BugReporter.exe\"";
#else
				lpFile = L"\"..\\bin\\Release\\x64\\BugReporter.exe\"";
#endif
			}
			else
			{
				lpFile = L"\"BugReporter.exe\"";
			}

			ShellExecuteW(NULL, lpOperation, lpFile, NULL, NULL, SW_SHOWDEFAULT);
		}

	private:
		BugReporter();;
		~BugReporter();;
	};
} // namespace BugReporter