#include "Core/Serialization/ISerializer.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldResource.h"
#include "Heightfield/MaterialMask.h"
#include "Heightfield/MaterialMaskResource.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.HeightfieldShapeDesc", 2, HeightfieldShapeDesc, ShapeDesc)

void HeightfieldShapeDesc::setHeightfield(const resource::Proxy< hf::Heightfield >& heightfield)
{
	m_heightfield = heightfield;
}

const resource::Proxy< hf::Heightfield >& HeightfieldShapeDesc::getHeightfield() const
{
	return m_heightfield;
}

void HeightfieldShapeDesc::setMaterialMask(const resource::Proxy< hf::MaterialMask >& materialMask)
{
	m_materialMask = materialMask;
}

const resource::Proxy< hf::MaterialMask >& HeightfieldShapeDesc::getMaterialMask() const
{
	return m_materialMask;
}

bool HeightfieldShapeDesc::serialize(ISerializer& s)
{
	if (!ShapeDesc::serialize(s))
		return false;

	s >> resource::Member< hf::Heightfield, hf::HeightfieldResource >(L"heightfield", m_heightfield);

	if (s.getVersion() >= 2)
		s >> resource::Member< hf::MaterialMask, hf::MaterialMaskResource >(L"materialMask", m_materialMask);

	return true;
}

	}
}
