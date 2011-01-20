#include "Amalgam/Editor/PlatformInstance.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.PlatformInstance", PlatformInstance, Object)

PlatformInstance::PlatformInstance(const std::wstring& name, const Platform* platform)
:	m_name(name)
,	m_platform(platform)
{
}

const std::wstring& PlatformInstance::getName() const
{
	return m_name;
}

const Platform* PlatformInstance::getPlatform() const
{
	return m_platform;
}

	}
}
