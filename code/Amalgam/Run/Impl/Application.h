/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_Application_H
#define traktor_amalgam_Application_H

#include "Amalgam/Run/IApplication.h"
#include "Core/RefArray.h"
#include "Core/Library/Library.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_RUN_EXPORT)
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

class ScriptServer;
class Environment;
class TargetManagerConnection;

/*! \brief Amalgam application implementation.
 * \ingroup Amalgam
 */
class T_DLLCLASS Application : public IApplication
{
	T_RTTI_CLASS;

public:
	Application();

	bool create(
		const PropertyGroup* defaultSettings,
		PropertyGroup* settings
	);

	void destroy();

	bool execute();

	virtual Ref< IEnvironment > getEnvironment();

private:
	Ref< PropertyGroup > m_settings;
	RefArray< Library > m_libraries;
	Ref< TargetManagerConnection > m_targetManagerConnection;
	Ref< db::Database > m_database;
	Ref< ScriptServer > m_scriptServer;
	Ref< Environment > m_environment;
};

	}
}

#endif	// traktor_amalgam_Application_H
