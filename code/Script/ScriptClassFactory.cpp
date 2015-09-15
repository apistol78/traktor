#include "Core/Class/IRuntimeClass.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Script/IScriptContext.h"
#include "Script/IScriptManager.h"
#include "Script/IScriptResource.h"
#include "Script/ScriptClassFactory.h"

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
	typeSet.insert(&type_of< IScriptResource >());
	return typeSet;
}

const TypeInfoSet ScriptClassFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IRuntimeClass >());
	return typeSet;
}

bool ScriptClassFactory::isCacheable() const
{
	return true;
}

Ref< Object > ScriptClassFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
	{
		log::error << L"Unable to create script class; no such instance" << Endl;
		return 0;
	}

	Ref< IScriptResource > scriptResource = instance->getObject< IScriptResource >();
	if (!scriptResource)
	{
		log::error << L"Unable to create script class; no such instance" << Endl;
		return 0;
	}

	if (!m_scriptContext->loadResource(scriptResource))
	{
		log::error << L"Unable to create script class; load resource failed" << Endl;
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

	}
}
