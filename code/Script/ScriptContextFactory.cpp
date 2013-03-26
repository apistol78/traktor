#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Script/IScriptContext.h"
#include "Script/IScriptManager.h"
#include "Script/IScriptResource.h"
#include "Script/ScriptContextFactory.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptContextFactory", ScriptContextFactory, resource::IResourceFactory)

ScriptContextFactory::ScriptContextFactory(db::Database* database, IScriptManager* scriptManager)
:	m_database(database)
,	m_scriptManager(scriptManager)
{
}

const TypeInfoSet ScriptContextFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IScriptContext >());
	return typeSet;
}

bool ScriptContextFactory::isCacheable() const
{
	return false;
}

Ref< Object > ScriptContextFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const
{
	Ref< IScriptResource > scriptResource = m_database->getObjectReadOnly< IScriptResource >(guid);
	if (!scriptResource)
	{
		log::error << L"Unable to create script context; no such instance" << Endl;
		return 0;
	}

	Ref< IScriptContext > scriptContext = m_scriptManager->createContext(scriptResource);
	if (!scriptContext)
	{
		log::error << L"Unable to create script context; create context failed" << Endl;
		return 0;
	}

	return scriptContext;
}

	}
}
