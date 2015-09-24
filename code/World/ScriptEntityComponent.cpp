#include "Core/Class/IRuntimeClass.h"
#include "World/Entity.h"
#include "World/ScriptEntityComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ScriptEntityComponent", ScriptEntityComponent, IEntityComponent)

ScriptEntityComponent::ScriptEntityComponent(Entity* owner, const resource::Proxy< IRuntimeClass >& clazz)
:	m_owner(owner)
,	m_class(clazz)
,	m_methodUpdate(~0U)
{
	// Invoke script class constructor.
	m_object = m_class->construct(m_owner, 0, 0);
	m_methodUpdate = findRuntimeClassMethodId(m_class, "update");
	m_class.consume();
}

void ScriptEntityComponent::update(const UpdateParams& update)
{
	// Check if class has changed, hot-reload new class.
	if (m_class.changed())
	{
		m_object = m_class->construct(m_owner, 0, 0);
		m_methodUpdate = findRuntimeClassMethodId(m_class, "update");
		m_class.consume();
	}

	// Invoke update method if available.
	if (m_class && m_object && m_methodUpdate != ~0U)
	{
		Any argv[] =
		{
			Any::fromFloat(update.totalTime),
			Any::fromFloat(update.deltaTime)
		};
		m_class->invoke(m_object, m_methodUpdate, sizeof_array(argv), argv);
	}
}

	}
}
