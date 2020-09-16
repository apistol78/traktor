#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Editor/Pipeline/AgentOpenFile.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.AgentOpenFile", 0, AgentOpenFile, ISerializable)

AgentOpenFile::AgentOpenFile(const Path& filePath)
:	m_filePath(filePath)
{
}

const Path& AgentOpenFile::getFilePath() const
{
	return m_filePath;
}

void AgentOpenFile::serialize(ISerializer& s)
{
	s >> Member< Path >(L"filePath", m_filePath);
}

	}
}
