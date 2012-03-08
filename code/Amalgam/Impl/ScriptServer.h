#ifndef traktor_amalgam_ScriptServer_H
#define traktor_amalgam_ScriptServer_H

#include "Amalgam/IScriptServer.h"

namespace traktor
{

class PropertyGroup;

	namespace script
	{

class IScriptDebugger;

	}

	namespace amalgam
	{

class IEnvironment;

class ScriptServer : public IScriptServer
{
	T_RTTI_CLASS;

public:
	bool create(const PropertyGroup* settings, bool debugger);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	void update();

	virtual script::IScriptManager* getScriptManager();

private:
	Ref< script::IScriptManager > m_scriptManager;
	Ref< script::IScriptDebugger > m_scriptDebugger;
};

	}
}

#endif	// traktor_amalgam_ScriptServer_H
