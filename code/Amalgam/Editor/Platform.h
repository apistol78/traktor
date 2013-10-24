#ifndef traktor_amalgam_Platform_H
#define traktor_amalgam_Platform_H

#include "Amalgam/Editor/DeployTool.h"
#include "Core/Guid.h"

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
	Platform();

	int32_t getIconIndex() const;

	const DeployTool& getDeployTool() const;

	virtual void serialize(ISerializer& s);

private:
	int32_t m_iconIndex;
	DeployTool m_deployToolWin32;
	DeployTool m_deployToolWin64;
	DeployTool m_deployToolOsX;
};

	}
}

#endif	// traktor_amalgam_Platform_H
