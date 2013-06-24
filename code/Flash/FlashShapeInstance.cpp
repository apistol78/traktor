#include "Flash/FlashShapeInstance.h"
#include "Flash/FlashShape.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashShapeInstance", FlashShapeInstance, FlashCharacterInstance)

FlashShapeInstance::FlashShapeInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashShape* shape)
:	FlashCharacterInstance(context, "Object", parent)
,	m_shape(shape)
{
}

const FlashShape* FlashShapeInstance::getShape() const
{
	return m_shape;
}

Aabb2 FlashShapeInstance::getBounds() const
{
	Aabb2 bounds = m_shape->getShapeBounds();
	bounds.mn = getTransform() * bounds.mn;
	bounds.mx = getTransform() * bounds.mx;
	return bounds;
}

	}
}
