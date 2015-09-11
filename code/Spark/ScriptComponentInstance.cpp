#include "Core/Class/IRuntimeClass.h"
#include "Script/IScriptContext.h"
#include "Spark/ScriptComponentInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ScriptComponentInstance", ScriptComponentInstance, IComponentInstance)

ScriptComponentInstance::ScriptComponentInstance(CharacterInstance* owner, const IRuntimeClass* scriptClass, ITypedObject* scriptObject)
:	m_owner(owner)
,	m_scriptClass(scriptClass)
,	m_scriptObject(scriptObject)
,	m_methodIdUpdate(findRuntimeClassMethodId(scriptClass, "update"))
{
}

void ScriptComponentInstance::update()
{
	if (m_methodIdUpdate != ~0U)
		m_scriptClass->invoke(m_scriptObject, m_methodIdUpdate, 0, 0);
}

	}
}
