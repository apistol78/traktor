#ifndef traktor_amalgam_BuildTargetAction_H
#define traktor_amalgam_BuildTargetAction_H

#include "Amalgam/Editor/Deploy/ITargetAction.h"
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

/*! \brief Build target action.
 * \ingroup Amalgam
 */
class T_DLLCLASS BuildTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	BuildTargetAction(
		db::Database* database,
		const PropertyGroup* globalSettings,
		const PropertyGroup* defaultPipelineSettings,
		const Target* target,
		const TargetConfiguration* targetConfiguration,
		const std::wstring& outputPath,
		bool standAlone
	);

	virtual bool execute(IProgressListener* progressListener) T_OVERRIDE T_FINAL;

private:
	Ref< db::Database > m_database;
	Ref< const PropertyGroup > m_globalSettings;
	Ref< const PropertyGroup > m_defaultPipelineSettings;
	Ref< const Target > m_target;
	Ref< const TargetConfiguration > m_targetConfiguration;
	std::wstring m_outputPath;
	bool m_standAlone;
};

	}
}

#endif	// traktor_amalgam_BuildTargetAction_H