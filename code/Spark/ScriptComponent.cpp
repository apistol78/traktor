#include "Core/Class/IRuntimeClass.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Script/IScriptContext.h"
#include "Spark/ScriptComponent.h"
#include "Spark/ScriptComponentInstance.h"
#include "Spark/StageInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.ScriptComponent", 0, ScriptComponent, IComponent)

Ref< IComponentInstance > ScriptComponent::createInstance(StageInstance* stage, CharacterInstance* owner, resource::IResourceManager* resourceManager) const
{
	script::IScriptContext* scriptContext = stage->getScriptContext();
	if (!scriptContext)
		return 0;

	Ref< const IRuntimeClass > scriptClass = scriptContext->findClass(m_class);
	if (!scriptClass)
		return 0;

	Ref< ITypedObject > scriptObject = scriptClass->construct(0, 0);
	if (!scriptObject)
		return 0;

	return new ScriptComponentInstance(owner, scriptClass, scriptObject);
}

void ScriptComponent::serialize(ISerializer& s)
{
	s >> Member< std::string >(L"class", m_class);
}

	}
}
