#pragma optimize( "", off )

#include "Core/Class/IRuntimeClass.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Script/IScriptContext.h"
#include "Spark/CharacterInstance.h"
#include "Spark/ScriptComponent.h"
#include "Spark/ScriptComponentInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.ScriptComponent", 0, ScriptComponent, IComponent)

Ref< IComponentInstance > ScriptComponent::createInstance(CharacterInstance* owner, resource::IResourceManager* resourceManager) const
{
	Ref< ScriptComponentInstance > instance = new ScriptComponentInstance(owner);
	if (resourceManager->bind(m_script, instance->m_script))
	{
		/*
		instance->m_script->setGlobal("character", Any::fromObject(owner));
		instance->m_script->executeFunction("setup", 0, 0);
		*/

		Ref< const IRuntimeClass > tigerClass = instance->m_script->findClass("Tiger");
		Ref< ITypedObject > tigerObject = tigerClass->construct(owner, 0, 0);


		tigerClass->invoke(
			tigerObject,
			findRuntimeClassMethodId(tigerClass, "update"),
			0,
			0
		);

		return instance;
	}
	else
		return 0;
}

void ScriptComponent::serialize(ISerializer& s)
{
	s >> resource::Member< script::IScriptContext >(L"script", m_script);
}

	}
}
