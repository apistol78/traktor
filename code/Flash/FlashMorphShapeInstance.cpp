#include "Flash/FlashMorphShape.h"
#include "Flash/FlashMorphShapeInstance.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMorphShapeInstance", FlashMorphShapeInstance, FlashCharacterInstance)

FlashMorphShapeInstance::FlashMorphShapeInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashMorphShape* shape)
:	FlashCharacterInstance(context, "Object", parent)
,	m_shape(shape)
{
}

const FlashMorphShape* FlashMorphShapeInstance::getShape() const
{
	return m_shape;
}

Aabb2 FlashMorphShapeInstance::getBounds() const
{
	Aabb2 bounds = m_shape->getShapeBounds();
	bounds.mn = getTransform() * bounds.mn;
	bounds.mx = getTransform() * bounds.mx;
	return bounds;
}

	}
}
