#include "Amalgam/Run/Impl/Environment.h"
#include "Amalgam/Run/Impl/ScriptServer.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.Environment", Environment, IEnvironment)

Environment::Environment(
	PropertyGroup* settings,
	db::Database* database,
	ScriptServer* scriptServer
)
:	m_settings(settings)
,	m_database(database)
,	m_scriptServer(scriptServer)
,	m_alive(true)
{
}

db::Database* Environment::getDatabase()
{
	return m_database;
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
