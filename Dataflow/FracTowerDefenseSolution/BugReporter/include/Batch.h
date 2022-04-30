#pragma once
#include <fstream>

namespace BugReporter
{
	class Batch
	{
	public:
		Batch();
		~Batch() = default;

		/**
		* Creates the batch file, fills it with the curl RestAPI calls, executes it and removes it
		*/
		void ExecuteBatchCommands();

	private:
		void CreateJiraIssueWithData();

		std::ofstream m_batch;
	};

} // namespace BugReporter