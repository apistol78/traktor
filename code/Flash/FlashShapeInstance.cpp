#include "Flash/FlashShapeInstance.h"
#include "Flash/FlashShape.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashShapeInstance", FlashShapeInstance, FlashCharacterInstance)

FlashShapeInstance::FlashShapeInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashShape* shape)
:	FlashCharacterInstance(context, L"Object", parent)
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
