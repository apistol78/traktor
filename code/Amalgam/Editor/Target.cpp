#include "Amalgam/Editor/Target.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.amalgam.Target", 0, Target, ISerializable)

const std::wstring& Target::getPipelineConfiguration() const
{
	return m_pipelineConfiguration;
}

const std::wstring& Target::getApplicationConfiguration() const
{
	return m_applicationConfiguration;
}

const std::wstring& Target::getExecutable() const
{
	return m_executable;
}

const Guid& Target::getRootAsset() const
{
	return m_rootAsset;
}

const Guid& Target::getStartupInstance() const
{
	return m_startupInstance;
}

bool Target::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"pipelineConfiguration", m_pipelineConfiguration);
	s >> Member< std::wstring >(L"applicationConfiguration", m_applicationConfiguration);
	s >> Member< std::wstring >(L"executable", m_executable);
	s >> Member< Guid >(L"rootAsset", m_rootAsset);
	s >> Member< Guid >(L"startupInstance", m_startupInstance);
	return true;
}

	}
}
