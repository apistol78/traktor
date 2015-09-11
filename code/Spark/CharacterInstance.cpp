#include "Spark/CharacterInstance.h"
#include "Spark/IComponentInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.CharacterInstance", CharacterInstance, Object)

CharacterInstance::CharacterInstance()
:	m_transform(Matrix33::identity())
{
}

void CharacterInstance::update()
{
	for (RefArray< IComponentInstance >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		(*i)->update();
}

void CharacterInstance::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

const Matrix33& CharacterInstance::getTransform() const
{
	return m_transform;
}

void CharacterInstance::addComponent(IComponentInstance* component)
{
	m_components.push_back(component);
}

const RefArray< IComponentInstance >& CharacterInstance::getComponents() const
{
	return m_components;
}

	}
}
