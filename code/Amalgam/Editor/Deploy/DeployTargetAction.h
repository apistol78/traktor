#ifndef traktor_amalgam_DeployTargetAction_H
#define traktor_amalgam_DeployTargetAction_H

#include "Amalgam/Editor/Deploy/ITargetAction.h"
#include "Core/Guid.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_DEPLOY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace db
	{

class Database;

	}

	namespace amalgam
	{

class Target;
class TargetConfiguration;

class T_DLLCLASS DeployTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	DeployTargetAction(
		db::Database* database,
		const PropertyGroup* globalSettings,
		const std::wstring& targetName,
		const Target* target,
		const TargetConfiguration* targetConfiguration,
		const std::wstring& editorHost,
		const std::wstring& deployHost,
		uint16_t databasePort,
		const std::wstring& databaseName,
		uint16_t targetManagerPort,
		const Guid& targetManagerId,
		const std::wstring& outputPath,
		const PropertyGroup* tweakSettings
	);

	virtual bool execute(IProgressListener* progressListener);

private:
	Ref< db::Database > m_database;
	Ref< const PropertyGroup > m_globalSettings;
	std::wstring m_targetName;
	Ref< const Target > m_target;
	Ref< const TargetConfiguration > m_targetConfiguration;
	std::wstring m_editorHost;
	std::wstring m_deployHost;
	uint16_t m_databasePort;
	std::wstring m_databaseName;
	uint16_t m_targetManagerPort;
	Guid m_targetManagerId;
	std::wstring m_outputPath;
	Ref< const PropertyGroup > m_tweakSettings;
};

	}
}

#endif	// traktor_amalgam_DeployTargetAction_H