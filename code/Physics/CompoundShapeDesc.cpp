#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Physics/CompoundShapeDesc.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.CompoundShapeDesc", 3, CompoundShapeDesc, ShapeDesc)

void CompoundShapeDesc::serialize(ISerializer& s)
{
	ShapeDesc::serialize(s);
	s >> MemberRefArray< ShapeDesc >(L"shapes", m_shapes);
}

	}
}
