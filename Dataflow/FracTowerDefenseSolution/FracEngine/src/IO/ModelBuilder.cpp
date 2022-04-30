#include "pch.h"
#include "Core/FileIO.h"
#include "IO/ModelBuilder.h"

namespace Frac
{
	ModelBuilder::ModelBuilder(Frac::Renderer& renderSystem, ResourceManager& resourceManager, const std::string& filePath) :
		m_resourceManager(resourceManager),
		m_renderSystem(renderSystem),
		m_ModelDirectory(""),
		m_ModelName(FileIO::GetNameFromFilePath(filePath))
	{
		const std::size_t extentionCaretPos = filePath.find_last_of('.');
		const std::size_t extentionStringLength = filePath.size() - extentionCaretPos;
		m_ModelDirectory = FileIO::GetPathFromWildcard("[Assets]") + filePath.substr(0, filePath.size() - m_ModelName.size() - extentionStringLength);
	}
}