#include "JsonParser.h"
#include <filesystem>
#include <string>
#include <fstream>

#include "Batch.h"

namespace BugReporter
{
	JsonParser::JsonParser() : m_data(nullptr) { }

	void JsonParser::SetBugReportData(char* a_summary, char* a_reproductionSteps, char* a_expectedOutcome, char* a_actualOutcome)
	{
		std::ifstream fracLogFile("Log/FracLog.txt");
		std::string defaultDescription = "This Bug Issue has been created using the in-engine bug tracker \n\n";
		std::string fracLogData((
			std::istreambuf_iterator<char>(fracLogFile)),
			std::istreambuf_iterator<char>());
		
		m_data = new nlohmann::json();

		nlohmann::json summary = {"summary", a_summary };
		nlohmann::json description = { "description", defaultDescription + fracLogData };

		nlohmann::json issueID = {"id", "10101"};
		nlohmann::json issueType = { "issuetype", {issueID} };

		nlohmann::json reproductionSteps = { "customfield_10301", a_reproductionSteps };
		nlohmann::json expectedOutcome = { "customfield_10303", a_expectedOutcome };
		nlohmann::json actualOutcome = { "customfield_10304", a_actualOutcome };

		nlohmann::json projectKey = { "key", "Y2021DF" };
		nlohmann::json project = { "project", {projectKey} };

		nlohmann::json labels = { "labels", {"programming"} };


		(*m_data)["fields"] = 
		{
			summary, 
			issueType,
			description,
			reproductionSteps,
			expectedOutcome,
			actualOutcome,
			project,
			labels
		};
	}
	void JsonParser::WriteDataToFile()
	{
		if (m_data)
		{
			std::string fileDirectory = std::filesystem::current_path().string() + "\\";
			std::string newFilePath = fileDirectory + "BugReportData.json";
			std::ofstream file(newFilePath);
			file << std::setw(4) << *m_data << std::endl;
			file.close();

			Batch batch;
			batch.ExecuteBatchCommands();

		}
	}
} // namespace BugReporter