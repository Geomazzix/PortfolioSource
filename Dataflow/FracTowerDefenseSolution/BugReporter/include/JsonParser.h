#pragma once
#include <Nlohmann/json.hpp>

namespace BugReporter 
{
	class JsonParser 
	{
	public:
		JsonParser();
		~JsonParser() = default;

		/**
		* Sets the required data according to the BugReporter environment input
		* @param a_summary: the 'Summary' input from the BugReporting environment
		* @param a_reproductionSteps: the 'Reproduction steps' input from the BugReporting environment
		* @param a_expectedOutcome: the 'Expected outcome' input from the BugReporting environment
		* @param a_actualOutcome: the 'Actual outcome' input from the BugReporting environment
		*/
		void SetBugReportData(char* a_summary, char* a_reproductionSteps, char* a_expectedOutcome, char* a_actualOutcome);

		/**
		* Creates a JSON file and Writes the data to the JSON file
		*/
		void WriteDataToFile();

	private:
		nlohmann::json* m_data;
	};
} // namespace BugReporter