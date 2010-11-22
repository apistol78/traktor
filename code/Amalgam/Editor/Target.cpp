#include "Amalgam/Editor/Target.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.amalgam.Target", 4, Target, ISerializable)

const std::wstring& Target::getDatabase() const
{
	return m_database;
}

const std::wstring& Target::getTargetPath() const
{
	return m_targetPath;
}

const std::wstring& Target::getDeployTool() const
{
	return m_deployTool;
}

const std::wstring& Target::getExecutable() const
{
	return m_executable;
}

const std::wstring& Target::getConfiguration() const
{
	return m_configuration;
}

const Guid& Target::getRootAsset() const
{
	return m_rootAsset;
}

bool Target::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 3)

	s >> Member< std::wstring >(L"database", m_database);
	s >> Member< std::wstring >(L"targetPath", m_targetPath);
	s >> Member< std::wstring >(L"deployTool", m_deployTool);
	s >> Member< std::wstring >(L"executable", m_executable);
	s >> Member< std::wstring >(L"configuration", m_configuration);

	if (s.getVersion() >= 4)
		s >> Member< Guid >(L"rootAsset", m_rootAsset);

	return true;
}

	}
}
