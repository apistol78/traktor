#include "Flash/FlashCharacterInstance.h"
#include "Flash/Action/Common/ColorTransform.h"
#include "Flash/Action/Common/Transform.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Transform", Transform, ActionObjectRelay)

Transform::Transform(FlashCharacterInstance* instance)
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
	m_instance->setColorTransform(colorTransform->getTransform());
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
