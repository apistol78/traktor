#ifndef traktor_amalgam_PlatformInstance_H
#define traktor_amalgam_PlatformInstance_H

#include "Core/Object.h"

namespace traktor
{
	namespace amalgam
	{

class Platform;

class PlatformInstance : public Object
{
	T_RTTI_CLASS;

public:
	PlatformInstance(const std::wstring& name, const Platform* platform);

	const std::wstring& getName() const;

	const Platform* getPlatform() const;

private:
	std::wstring m_name;
	Ref< const Platform > m_platform;
};

	}
}

#endif	// traktor_amalgam_PlatformInstance_H
