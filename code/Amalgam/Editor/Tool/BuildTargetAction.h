#ifndef traktor_amalgam_BuildTargetAction_H
#define traktor_amalgam_BuildTargetAction_H

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

class BuildTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	BuildTargetAction(
		db::Database* database,
		const PropertyGroup* globalSettings,
		const PropertyGroup* defaultPipelineSettings,
		const Target* target,
		const TargetConfiguration* targetConfiguration,
		const std::wstring& outputPath
	);

	virtual bool execute(IProgressListener* progressListener);

private:
	Ref< db::Database > m_database;
	Ref< const PropertyGroup > m_globalSettings;
	Ref< const PropertyGroup > m_defaultPipelineSettings;
	Ref< const Target > m_target;
	Ref< const TargetConfiguration > m_targetConfiguration;
	std::wstring m_outputPath;
};

	}
}

#endif	// traktor_amalgam_BuildTargetAction_H