#include "Amalgam/Editor/Platform.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.amalgam.Platform", 1, Platform, ISerializable)

const std::wstring& Platform::getPipelineConfiguration() const
{
	return m_pipelineConfiguration;
}

const std::wstring& Platform::getApplicationConfiguration() const
{
	return m_applicationConfiguration;
}

const std::wstring& Platform::getDeployTool() const
{
#if TARGET_OS_MAC
	return m_deployToolOsX;
#else
	return m_deployTool;
#endif
}

bool Platform::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"pipelineConfiguration", m_pipelineConfiguration);
	s >> Member< std::wstring >(L"applicationConfiguration", m_applicationConfiguration);
	s >> Member< std::wstring >(L"deployTool", m_deployTool);
	if (s.getVersion() >= 1)
		s >> Member< std::wstring >(L"deployToolOsX", m_deployToolOsX);
	return true;
}

	}
}
