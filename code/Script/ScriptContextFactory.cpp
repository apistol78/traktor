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
	type_of< IScriptResource >().findAllOf(typeSet);
	return typeSet;
}

const TypeInfoSet ScriptContextFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	type_of< IScriptContext >().findAllOf(typeSet);
	return typeSet;
}

bool ScriptContextFactory::isCacheable() const
{
	return false;
}

Ref< Object > ScriptContextFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	Ref< IScriptResource > scriptResource = m_database->getObjectReadOnly< IScriptResource >(guid);
	if (!scriptResource)
	{
		log::error << L"Unable to create script context; no such instance" << Endl;
		return 0;
	}

	Ref< IScriptContext > scriptContext = m_scriptManager->createContext(scriptResource, checked_type_cast< const IScriptContext* >(current));
	if (!scriptContext)
	{
		log::error << L"Unable to create script context; create context failed" << Endl;
		return 0;
	}

	return scriptContext;
}

	}
}
