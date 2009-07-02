#include "Script/ScriptContextFactory.h"
#include "Script/ScriptManager.h"
#include "Script/ScriptContext.h"
#include "Script/Script.h"
#include "Database/Database.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptContextFactory", ScriptContextFactory, resource::IResourceFactory)

ScriptContextFactory::ScriptContextFactory(db::Database* database, ScriptManager* scriptManager)
:	m_database(database)
,	m_scriptManager(scriptManager)
{
}

const TypeSet ScriptContextFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< ScriptContext >());
	return typeSet;
}

bool ScriptContextFactory::isCacheable() const
{
	return false;
}

Object* ScriptContextFactory::create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid)
{
	Ref< Script > s = m_database->getObjectReadOnly< Script >(guid);
	if (!s)
	{
		log::error << L"Unable to create script context; no such instance" << Endl;
		return 0;
	}

	Ref< ScriptContext > scriptContext = m_scriptManager->createContext();
	if (!scriptContext)
	{
		log::error << L"Unable to create script context; create context failed" << Endl;
		return 0;
	}

	if (!scriptContext->executeScript(s->getText(), false, 0))
	{
		log::error << L"Unable to create script context; execute script failed" << Endl;
		return 0;
	}

	return scriptContext;
}

	}
}
