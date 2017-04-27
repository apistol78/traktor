/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_LaunchTargetAction_H
#define traktor_amalgam_LaunchTargetAction_H

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

/*! \brief Launch target action.
 * \ingroup Amalgam
 */
class T_DLLCLASS LaunchTargetAction : public ITargetAction
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

	virtual bool execute(IProgressListener* progressListener) T_OVERRIDE T_FINAL;

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