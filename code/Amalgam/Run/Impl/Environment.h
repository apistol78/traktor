#ifndef traktor_amalgam_Environment_H
#define traktor_amalgam_Environment_H

#include "Amalgam/Run/IEnvironment.h"

namespace traktor
{
	namespace amalgam
	{

class ScriptServer;

class Environment : public IEnvironment
{
	T_RTTI_CLASS;

public:
	Environment(
		PropertyGroup* settings,
		db::Database* database,
		ScriptServer* scriptServer
	);

	virtual db::Database* getDatabase();

	virtual IScriptServer* getScript();

	virtual PropertyGroup* getSettings();

private:
	Ref< PropertyGroup > m_settings;
	Ref< db::Database > m_database;
	Ref< ScriptServer > m_scriptServer;
};

	}
}

#endif	// traktor_amalgam_Environment_H
