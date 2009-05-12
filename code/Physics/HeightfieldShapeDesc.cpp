#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/Heightfield.h"
#include "Physics/HeightfieldResource.h"
#include "Core/Serialization/Serializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.HeightfieldShapeDesc", HeightfieldShapeDesc, ShapeDesc)

void HeightfieldShapeDesc::setHeightfield(const resource::Proxy< Heightfield >& heightfield)
{
	m_heightfield = heightfield;
}

const resource::Proxy< Heightfield >& HeightfieldShapeDesc::getHeightfield() const
{
	return m_heightfield;
}

bool HeightfieldShapeDesc::serialize(Serializer& s)
{
	if (!ShapeDesc::serialize(s))
		return false;

	return s >> resource::Member< Heightfield, HeightfieldResource >(L"heightfield", m_heightfield);
}

	}
}
