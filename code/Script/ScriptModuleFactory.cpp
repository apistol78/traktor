#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Script/IScriptContext.h"
#include "Script/IScriptManager.h"
#include "Script/IScriptResource.h"
#include "Script/ScriptModuleFactory.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptModuleFactory", ScriptModuleFactory, resource::IResourceFactory)

ScriptModuleFactory::ScriptModuleFactory(db::Database* database, IScriptManager* scriptManager)
:	m_database(database)
,	m_scriptManager(scriptManager)
{
}

const TypeInfoSet ScriptModuleFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IScriptResource >());
	return typeSet;
}

const TypeInfoSet ScriptModuleFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	type_of< IScriptContext >().findAllOf(typeSet);
	return typeSet;
}

bool ScriptModuleFactory::isCacheable() const
{
	return false;
}

Ref< Object > ScriptModuleFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	Ref< IScriptResource > scriptResource = m_database->getObjectReadOnly< IScriptResource >(guid);
	if (!scriptResource)
	{
		log::error << L"Unable to create script context; no such instance" << Endl;
		return 0;
	}

	Ref< IScriptContext > scriptContext = m_scriptManager->createContext();
	if (!scriptContext)
	{
		log::error << L"Unable to create script context; create context failed" << Endl;
		return 0;
	}

	if (!scriptContext->loadResource(scriptResource))
	{
		log::error << L"Unable to create script context; load resource failed" << Endl;
		return 0;
	}

	return scriptContext;
}

	}
}
