#include "Spark/MorphShape.h"
#include "Spark/MorphShapeInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.MorphShapeInstance", MorphShapeInstance, CharacterInstance)

MorphShapeInstance::MorphShapeInstance(Context* context, Dictionary* dictionary, CharacterInstance* parent, const MorphShape* shape)
:	CharacterInstance(context, dictionary, parent)
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
