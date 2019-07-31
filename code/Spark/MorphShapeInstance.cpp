#include "Spark/MorphShape.h"
#include "Spark/MorphShapeInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.MorphShapeInstance", MorphShapeInstance, CharacterInstance)

MorphShapeInstance::MorphShapeInstance(ActionContext* context, Dictionary* dictionary, CharacterInstance* parent, const MorphShape* shape)
:	CharacterInstance(context, "Object", dictionary, parent)
,	m_shape(shape)
{
}

const MorphShape* MorphShapeInstance::getShape() const
{
	return m_shape;
}

Aabb2 MorphShapeInstance::getBounds() const
{
	return getTransform() * m_shape->getShapeBounds();
}

	}
}
