#include "Core/Class/IRuntimeClass.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Resource/IResourceManager.h"
#include "Script/IScriptContext.h"
#include "Script/IScriptManager.h"
#include "Script/ScriptChunk.h"
#include "Script/ScriptClassFactory.h"
#include "Script/ScriptResource.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptClassFactory", ScriptClassFactory, resource::IResourceFactory)

ScriptClassFactory::ScriptClassFactory(db::Database* database, IScriptContext* scriptContext)
:	m_database(database)
,	m_scriptContext(scriptContext)
{
}

const TypeInfoSet ScriptClassFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ScriptResource >());
	return typeSet;
}

const TypeInfoSet ScriptClassFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IRuntimeClass >());
	typeSet.insert(&type_of< ScriptChunk >());
	return typeSet;
}

bool ScriptClassFactory::isCacheable() const
{
	return true;
}

Ref< Object > ScriptClassFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	if (is_type_a< IRuntimeClass >(resourceType))
	{
		Ref< resource::ResourceHandle > chunkHandle = resourceManager->bind(type_of< ScriptChunk >(), guid);
		if (!chunkHandle)
			return 0;

		Ref< db::Instance > instance = m_database->getInstance(guid);
		if (!instance)
		{
			log::error << L"Unable to create script class; no such instance" << Endl;
			return 0;
		}

		Ref< const IRuntimeClass > scriptClass = m_scriptContext->findClass(wstombs(instance->getName()));
		if (!scriptClass)
		{
			log::error << L"Unable to create script class; no such class \"" << instance->getName() << L"\"" << Endl;
			return 0;
		}

		return const_cast< IRuntimeClass* >(scriptClass.ptr());
	}
	else if (is_type_a< ScriptChunk >(resourceType))
	{
		Ref< ScriptResource > scriptResource = m_database->getObjectReadOnly< ScriptResource >(guid);
		if (!scriptResource)
		{
			log::error << L"Unable to create script class; no such instance" << Endl;
			return 0;
		}

		const std::vector< Guid >& dependencies = scriptResource->getDependencies();
		for (std::vector< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		{
			Ref< resource::ResourceHandle > chunkHandle = resourceManager->bind(type_of< ScriptChunk >(), *i);
			if (!chunkHandle)
				return 0;
		}

		if (!m_scriptContext->load(scriptResource->getBlob()))
		{
			log::error << L"Unable to create script class; load resource failed" << Endl;
			return 0;
		}

		return new Object();
	}
	else
		return 0;
}

	}
}
