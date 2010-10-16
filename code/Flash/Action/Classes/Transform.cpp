#include "Flash/FlashCharacterInstance.h"
#include "Flash/Action/Classes/ColorTransform.h"
#include "Flash/Action/Classes/Transform.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Transform", Transform, ActionObject)

Transform::Transform(FlashCharacterInstance* instance)
:	ActionObject("flash.geom.Transform")
,	m_instance(instance)
{
}

Ref< ColorTransform > Transform::getColorTransform() const
{
	return new ColorTransform(m_instance->getColorTransform());
}

void Transform::setColorTransform(const ColorTransform* colorTransform)
{
	m_instance->setColorTransform(colorTransform->getTransform());
}

void Transform::trace(const IVisitor& visitor) const
{
	visitor(m_instance);
	ActionObject::trace(visitor);
}

void Transform::dereference()
{
	m_instance = 0;
	ActionObject::dereference();
}

	}
}
