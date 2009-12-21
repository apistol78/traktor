#include "Core/Serialization/ISerializer.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/Heightfield.h"
#include "Physics/HeightfieldResource.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.HeightfieldShapeDesc", 1, HeightfieldShapeDesc, ShapeDesc)

void HeightfieldShapeDesc::setHeightfield(const resource::Proxy< Heightfield >& heightfield)
{
	m_heightfield = heightfield;
}

const resource::Proxy< Heightfield >& HeightfieldShapeDesc::getHeightfield() const
{
	return m_heightfield;
}

bool HeightfieldShapeDesc::bind(resource::IResourceManager* resourceManager)
{
	return resourceManager->bind(m_heightfield);
}

bool HeightfieldShapeDesc::serialize(ISerializer& s)
{
	if (!ShapeDesc::serialize(s))
		return false;

	return s >> resource::Member< Heightfield, HeightfieldResource >(L"heightfield", m_heightfield);
}

	}
}
