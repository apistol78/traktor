#include "Core/Class/IRuntimeClass.h"
#include "Spark/CharacterInstance.h"
#include "Spark/ScriptComponentInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ScriptComponentInstance", ScriptComponentInstance, IComponentInstance)

ScriptComponentInstance::ScriptComponentInstance(CharacterInstance* owner, const resource::Proxy< IRuntimeClass >& clazz)
:	m_owner(owner)
,	m_class(clazz)
,	m_methodUpdate(~0U)
{
	m_object = m_class->construct(m_owner, 0, 0);
	m_methodUpdate = findRuntimeClassMethodId(m_class, "update");
	m_class.consume();
}

void ScriptComponentInstance::update()
{
	if (m_class.changed())
	{
		m_object = m_class->construct(m_owner, 0, 0);
		m_methodUpdate = findRuntimeClassMethodId(m_class, "update");
		m_class.consume();
	}

	if (m_class && m_object)
		m_class->invoke(m_object, m_methodUpdate, 0, 0);
}

	}
}
