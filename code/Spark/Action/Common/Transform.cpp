#include "Spark/CharacterInstance.h"
#include "Spark/Action/Common/Transform.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Transform", Transform, ActionObjectRelay)

Transform::Transform(CharacterInstance* instance)
:	ActionObjectRelay("flash.geom.Transform")
,	m_instance(instance)
{
}

Ref< ColorTransform > Transform::getColorTransform() const
{
	return new ColorTransform(m_instance->getColorTransform());
}

void Transform::setColorTransform(const ColorTransform* colorTransform)
{
	m_instance->setColorTransform(*colorTransform);
}

void Transform::trace(visitor_t visitor) const
{
	visitor(m_instance);
	ActionObjectRelay::trace(visitor);
}

void Transform::dereference()
{
	m_instance = 0;
	ActionObjectRelay::dereference();
}

	}
}
