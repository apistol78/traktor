#include "Physics/BodyDesc.h"
#include "Physics/ShapeDesc.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BodyDesc", BodyDesc, Serializable)

void BodyDesc::setShape(ShapeDesc* shape)
{
	m_shape = shape;
}

const Ref< ShapeDesc >& BodyDesc::getShape() const
{
	return m_shape;
}

bool BodyDesc::serialize(Serializer& s)
{
	return s >> MemberRef< ShapeDesc >(L"shape", m_shape);
}

	}
}
