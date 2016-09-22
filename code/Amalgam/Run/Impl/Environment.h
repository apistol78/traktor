#ifndef traktor_amalgam_Environment_H
#define traktor_amalgam_Environment_H

#include "Amalgam/Run/IEnvironment.h"

namespace traktor
{
	namespace amalgam
	{

class ScriptServer;

/*! \brief
 * \ingroup Amalgam
 */
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

	virtual bool alive() const;

private:
	friend class Application;

	Ref< PropertyGroup > m_settings;
	Ref< db::Database > m_database;
	Ref< ScriptServer > m_scriptServer;
	bool m_alive;
};

	}
}

#endif	// traktor_amalgam_Environment_H
