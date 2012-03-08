#ifndef traktor_amalgam_LaunchTargetAction_H
#define traktor_amalgam_LaunchTargetAction_H

#include "Amalgam/Editor/Tool/ITargetAction.h"
#include "Core/Ref.h"

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

class LaunchTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	LaunchTargetAction(
		db::Database* database,
		const PropertyGroup* globalSettings,
		const std::wstring& targetName,
		const Target* target,
		const TargetConfiguration* targetConfiguration,
		const std::wstring& deployHost,
		const std::wstring& outputPath
	);

	virtual bool execute(IProgressListener* progressListener);

private:
	Ref< db::Database > m_database;
	Ref< const PropertyGroup > m_globalSettings;
	std::wstring m_targetName;
	Ref< const Target > m_target;
	Ref< const TargetConfiguration > m_targetConfiguration;
	std::wstring m_deployHost;
	std::wstring m_outputPath;
};

	}
}

#endif	// traktor_amalgam_LaunchTargetAction_H