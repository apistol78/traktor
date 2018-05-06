/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_Environment_H
#define traktor_amalgam_Environment_H

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

	virtual db::Database* getDatabase() T_OVERRIDE T_FINAL;

	virtual IResourceServer* getResource() T_OVERRIDE T_FINAL;

	virtual IScriptServer* getScript() T_OVERRIDE T_FINAL;

	virtual PropertyGroup* getSettings() T_OVERRIDE T_FINAL;

	virtual bool alive() const T_OVERRIDE T_FINAL;

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

#endif	// traktor_amalgam_Environment_H
