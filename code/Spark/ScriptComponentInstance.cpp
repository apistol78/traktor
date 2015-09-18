#include "Core/Class/IRuntimeClass.h"
#include "Core/Misc/TString.h"
#include "Spark/ScriptComponentInstance.h"
#include "Spark/SpriteInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ScriptComponentInstance", ScriptComponentInstance, IComponentInstance)

ScriptComponentInstance::ScriptComponentInstance(SpriteInstance* owner, const resource::Proxy< IRuntimeClass >& clazz)
:	m_owner(owner)
,	m_class(clazz)
,	m_methodUpdate(~0U)
{
	IRuntimeClass::prototype_t proto;

	// Place all existing child characters in prototype to be accessible from constructor.
	RefArray< CharacterInstance > characters;
	m_owner->getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
	{
		if (!(*i)->getName().empty())
			proto[wstombs((*i)->getName())] = Any::fromObject(*i);
	}

	// Invoke script class constructor.
	m_object = m_class->construct(m_owner, 0, 0, proto);
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
