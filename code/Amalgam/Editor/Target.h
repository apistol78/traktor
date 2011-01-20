#ifndef traktor_amalgam_Target_H
#define traktor_amalgam_Target_H

#include "Core/Guid.h"
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

class T_DLLCLASS Target : public ISerializable
{
	T_RTTI_CLASS;

public:
	const std::wstring& getPipelineConfiguration() const;

	const std::wstring& getApplicationConfiguration() const;

	const std::wstring& getExecutable() const;

	const Guid& getRootAsset() const;

	const Guid& getStartupInstance() const;

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_pipelineConfiguration;
	std::wstring m_applicationConfiguration;
	std::wstring m_executable;
	Guid m_rootAsset;
	Guid m_startupInstance;
};

	}
}

#endif	// traktor_amalgam_Target_H
