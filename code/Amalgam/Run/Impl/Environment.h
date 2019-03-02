#pragma once

#include "Amalgam/Run/IEnvironment.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace amalgam
	{

class ResourceServer;
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
		ResourceServer* resourceServer,
		ScriptServer* scriptServer
	);

	virtual db::Database* getDatabase() override final;

	virtual IResourceServer* getResource() override final;

	virtual IScriptServer* getScript() override final;

	virtual PropertyGroup* getSettings() override final;

	virtual bool alive() const override final;

private:
	friend class Application;

	Ref< PropertyGroup > m_settings;
	Ref< db::Database > m_database;
	Ref< ResourceServer > m_resourceServer;
	Ref< ScriptServer > m_scriptServer;
	bool m_alive;
};

	}
}

