#include "Batch.h"
#include <iostream>
#include <stdlib.h>
#include <cstdio>
#include <filesystem>

namespace BugReporter
{
	Batch::Batch() : m_batch() { }

	void Batch::ExecuteBatchCommands()
	{
		std::string fileDirectory = std::filesystem::current_path().string() + "\\";
		std::string filePath = fileDirectory + "JiraIssueCreator.bat";

		//Create & open the Batch file
		m_batch.open(filePath, std::ios::out);
		
		CreateJiraIssueWithData();

		//Close the Batch file
		m_batch.close();

		//Execute the Batch file
		system(("\"" + filePath + "\"").c_str());

		//Remove the Batch file
		std::remove(filePath.c_str());
	}

	void Batch::CreateJiraIssueWithData()
	{
		m_batch << "curl ^\n";
		m_batch << "   -D- ^\n";
		m_batch << "   -H \"Authorization: Basic MTkxMDA1Ok8ySlFLMA==\" ^\n";
		m_batch << "   -X POST ^\n";
		m_batch << "   --data @BugReportData.json ^\n";
		m_batch << "   -H \"Content-Type: application/json\" ^\n";
		m_batch << "   https://jira1.buas.nl:8443/rest/api/2/issue/";
	}
} // namespace BugReporter
