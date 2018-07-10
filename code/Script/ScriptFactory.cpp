/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/IRuntimeClass.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Resource/IResourceManager.h"
#include "Script/IScriptContext.h"
#include "Script/ScriptChunk.h"
#include "Script/ScriptFactory.h"
#include "Script/ScriptResource.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptFactory", ScriptFactory, resource::IResourceFactory)

ScriptFactory::ScriptFactory(IScriptContext* scriptContext)
:	m_scriptContext(scriptContext)
{
}

const TypeInfoSet ScriptFactory::getResourceTypes() const
{
	return makeTypeInfoSet< ScriptResource >();
}

const TypeInfoSet ScriptFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< IRuntimeClass, ScriptChunk >();
}

bool ScriptFactory::isCacheable(const TypeInfo& productType) const
{
	if (is_type_a< IRuntimeClass >(productType))
		return false;
	else if (is_type_a< ScriptChunk >(productType))
		return true;
	else
		return false;
}

Ref< Object > ScriptFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	if (is_type_a< IRuntimeClass >(productType))
	{
		Ref< resource::ResourceHandle > chunkHandle = resourceManager->bind(type_of< ScriptChunk >(), instance->getGuid());
		if (!chunkHandle)
			return 0;

		Ref< const IRuntimeClass > scriptClass = m_scriptContext->findClass(wstombs(instance->getName()));
		if (!scriptClass)
		{
			log::error << L"Unable to create script class; no such class \"" << instance->getName() << L"\"" << Endl;
			return 0;
		}

		return const_cast< IRuntimeClass* >(scriptClass.ptr());
	}
	else if (is_type_a< ScriptChunk >(productType))
	{
		Ref< ScriptResource > scriptResource = instance->getObject< ScriptResource >();
		if (!scriptResource)
		{
			log::error << L"Unable to create script class; incorrect instance type." << Endl;
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
			log::error << L"Unable to create script class; load resource failed." << Endl;
			return 0;
		}

		return new Object();
	}
	else
		return 0;
}

	}
}
