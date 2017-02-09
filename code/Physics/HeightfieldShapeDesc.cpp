#include "Core/Serialization/ISerializer.h"
#include "Heightfield/Heightfield.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.HeightfieldShapeDesc", ShapeDesc::Version, HeightfieldShapeDesc, ShapeDesc)

void HeightfieldShapeDesc::setHeightfield(const resource::Id< hf::Heightfield >& heightfield)
{
	m_heightfield = heightfield;
}

const resource::Id< hf::Heightfield >& HeightfieldShapeDesc::getHeightfield() const
{
	return m_heightfield;
}

void HeightfieldShapeDesc::serialize(ISerializer& s)
{
	ShapeDesc::serialize(s);

	s >> resource::Member< hf::Heightfield >(L"heightfield", m_heightfield);
}

	}
}
