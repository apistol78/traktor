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
	const std::wstring& getDatabase() const;

	const std::wstring& getTargetPath() const;

	const std::wstring& getDeployTool() const;

	const std::wstring& getExecutable() const;

	const std::wstring& getConfiguration() const;

	const Guid& getRootAsset() const;

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_database;
	std::wstring m_targetPath;
	std::wstring m_deployTool;
	std::wstring m_executable;
	std::wstring m_configuration;
	Guid m_rootAsset;
};

	}
}

#endif	// traktor_amalgam_Target_H
