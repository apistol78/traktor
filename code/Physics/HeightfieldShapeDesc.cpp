#include "Core/Serialization/ISerializer.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldResource.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.HeightfieldShapeDesc", 1, HeightfieldShapeDesc, ShapeDesc)

void HeightfieldShapeDesc::setHeightfield(const resource::Proxy< hf::Heightfield >& heightfield)
{
	m_heightfield = heightfield;
}

const resource::Proxy< hf::Heightfield >& HeightfieldShapeDesc::getHeightfield() const
{
	return m_heightfield;
}

bool HeightfieldShapeDesc::serialize(ISerializer& s)
{
	if (!ShapeDesc::serialize(s))
		return false;

	return s >> resource::Member< hf::Heightfield, hf::HeightfieldResource >(L"heightfield", m_heightfield);
}

	}
}
