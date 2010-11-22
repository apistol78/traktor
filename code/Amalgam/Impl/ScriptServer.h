#ifndef traktor_amalgam_ScriptServer_H
#define traktor_amalgam_ScriptServer_H

#include "Amalgam/IScriptServer.h"

namespace traktor
{

class Settings;

	namespace amalgam
	{

class IEnvironment;

class ScriptServer : public IScriptServer
{
	T_RTTI_CLASS;

public:
	bool create(const Settings* settings);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const Settings* settings);

	virtual script::IScriptManager* getScriptManager();

private:
	Ref< script::IScriptManager > m_scriptManager;
};

	}
}

#endif	// traktor_amalgam_ScriptServer_H
