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

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptContextFactory", ScriptContextFactory, resource::ResourceFactory)

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

Object* ScriptContextFactory::create(const Type& resourceType, const Guid& guid, bool& outCacheable)
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

	if (!scriptContext->executeScript(s->getText()))
	{
		log::error << L"Unable to create script context; execute script failed" << Endl;
		return 0;
	}

	// Don't cache script contexts; each script context have their own local store.
	outCacheable = false;

	return scriptContext;
}

	}
}
