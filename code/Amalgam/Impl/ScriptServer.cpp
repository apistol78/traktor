#include "Amalgam/IEnvironment.h"
#include "Amalgam/Impl/LibraryHelper.h"
#include "Amalgam/Impl/ScriptServer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Resource/IResourceManager.h"
#include "Script/IScriptManager.h"
#include "Script/ScriptContextFactory.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ScriptServer", ScriptServer, IScriptServer)

bool ScriptServer::create(const Settings* settings, bool debugger)
{
	std::wstring scriptType = settings->getProperty< PropertyString >(L"Script.Type");

	m_scriptManager = loadAndInstantiate< script::IScriptManager >(scriptType);
	if (!m_scriptManager)
		return false;

	if (debugger)
	{
		m_scriptDebugger = m_scriptManager->createDebugger();
		if (!m_scriptDebugger)
			log::warning << L"Unable to create script debugger" << Endl;
	}

	return true;
}

void ScriptServer::destroy()
{
	m_scriptDebugger = 0;
	safeDestroy(m_scriptManager);
}

void ScriptServer::createResourceFactories(IEnvironment* environment)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	db::Database* database = environment->getDatabase();

	resourceManager->addFactory(new script::ScriptContextFactory(database, m_scriptManager));
}

int32_t ScriptServer::reconfigure(const Settings* settings)
{
	return CrUnaffected;
}

void ScriptServer::update()
{
	if (m_scriptManager)
		m_scriptManager->collectGarbage();
}

script::IScriptManager* ScriptServer::getScriptManager()
{
	return m_scriptManager;
}

	}
}
