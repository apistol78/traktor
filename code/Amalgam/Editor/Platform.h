#ifndef traktor_amalgam_Platform_H
#define traktor_amalgam_Platform_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class T_DLLCLASS Platform : public ISerializable
{
	T_RTTI_CLASS;

public:
	const std::wstring& getPipelineConfiguration() const;

	const std::wstring& getApplicationConfiguration() const;

	const std::wstring& getDeployTool() const;

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_pipelineConfiguration;
	std::wstring m_applicationConfiguration;
	std::wstring m_deployTool;
	std::wstring m_deployToolOsX;
};

	}
}

#endif	// traktor_amalgam_Platform_H
