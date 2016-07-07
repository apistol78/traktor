#include "Core/Class/IRuntimeClass.h"
#include "World/Entity.h"
#include "World/Entity/ScriptComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ScriptComponent", ScriptComponent, IEntityComponent)

ScriptComponent::ScriptComponent(const resource::Proxy< IRuntimeClass >& clazz)
:	m_owner(0)
,	m_class(clazz)
,	m_methodUpdate(~0U)
{
}

void ScriptComponent::destroy()
{
	m_owner = 0;
	m_class.clear();
	m_object = 0;
	m_methodUpdate = ~0U;
}

void ScriptComponent::setOwner(Entity* owner)
{
	T_ASSERT (m_owner == 0);
	m_owner = owner;
	m_object = m_class->construct(m_owner, 0, 0);
	m_methodUpdate = findRuntimeClassMethodId(m_class, "update");
	m_class.consume();
}

void ScriptComponent::setTransform(const Transform& transform)
{
}

Aabb3 ScriptComponent::getBoundingBox() const
{
	return Aabb3();
}

void ScriptComponent::update(const UpdateParams& update)
{
	T_ASSERT (m_owner != 0);

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
