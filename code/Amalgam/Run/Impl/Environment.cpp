/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Run/Impl/Environment.h"
#include "Amalgam/Run/Impl/ResourceServer.h"
#include "Amalgam/Run/Impl/ScriptServer.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.Environment", Environment, IEnvironment)

Environment::Environment(
	PropertyGroup* settings,
	db::Database* database,
	ResourceServer* resourceServer,
	ScriptServer* scriptServer
)
:	m_settings(settings)
,	m_database(database)
,	m_resourceServer(resourceServer)
,	m_scriptServer(scriptServer)
,	m_alive(true)
{
}

db::Database* Environment::getDatabase()
{
	return m_database;
}

IResourceServer* Environment::getResource()
{
	return m_resourceServer;
}

IScriptServer* Environment::getScript()
{
	return m_scriptServer;
}

PropertyGroup* Environment::getSettings()
{
	return m_settings;
}

bool Environment::alive() const
{
	return m_alive;
}

	}
}
