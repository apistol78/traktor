#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Editor/Pipeline/AgentOpenFile.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.AgentOpenFile", 0, AgentOpenFile, ISerializable)

AgentOpenFile::AgentOpenFile()
{
}

AgentOpenFile::AgentOpenFile(const Path& basePath, const std::wstring& fileName)
:	m_basePath(basePath)
,	m_fileName(fileName)
{
}

const Path& AgentOpenFile::getBasePath() const
{
	return m_basePath;
}

const std::wstring& AgentOpenFile::getFileName() const
{
	return m_fileName;
}

bool AgentOpenFile::serialize(ISerializer& s)
{
	s >> Member< Path >(L"basePath", m_basePath);
	s >> Member< std::wstring >(L"host", m_fileName);
	return true;
}

	}
}
