#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Script/IScriptContext.h"
#include "Script/IScriptManager.h"
#include "Script/ScriptModuleFactory.h"
#include "Script/ScriptResource.h"

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
	typeSet.insert(&type_of< ScriptResource >());
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
	Ref< ScriptResource > scriptResource = m_database->getObjectReadOnly< ScriptResource >(guid);
	if (!scriptResource)
	{
		log::error << L"Unable to create script context; no such instance" << Endl;
		return 0;
	}

	Ref< IScriptContext > scriptContext = m_scriptManager->createContext(true);
	if (!scriptContext)
	{
		log::error << L"Unable to create script context; create context failed" << Endl;
		return 0;
	}

	// Load all dependencies first.
	const std::vector< Guid >& dependencies = scriptResource->getDependencies();
	for (std::vector< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		Ref< ScriptResource > dependentScriptResource = m_database->getObjectReadOnly< ScriptResource >(*i);
		if (!dependentScriptResource)
		{
			log::error << L"Unable to create script context; failed to load dependent script" << Endl;
			return 0;
		}
		if (!scriptContext->load(dependentScriptResource->getBlob()))
		{
			log::error << L"Unable to create script context; load dependent resource failed" << Endl;
			return 0;
		}
	}

	// Load this resource's blob last.
	if (!scriptContext->load(scriptResource->getBlob()))
	{
		log::error << L"Unable to create script context; load resource failed" << Endl;
		return 0;
	}

	return scriptContext;
}

	}
}
