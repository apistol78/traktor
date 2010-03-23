#include "Flash/FlashShapeInstance.h"
#include "Flash/FlashShape.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashShapeInstance", FlashShapeInstance, FlashCharacterInstance)

FlashShapeInstance::FlashShapeInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashShape* shape)
:	FlashCharacterInstance(context, AsObject::getInstance(), parent)
,	m_shape(shape)
{
}

const FlashShape* FlashShapeInstance::getShape() const
{
	return m_shape;
}

SwfRect FlashShapeInstance::getBounds() const
{
	SwfRect bounds = m_shape->getShapeBounds();
	bounds.min = getTransform() * bounds.min;
	bounds.max = getTransform() * bounds.max;
	return bounds;
}

	}
}
