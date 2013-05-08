#include "Core/Serialization/ISerializer.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/MaterialMask.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.HeightfieldShapeDesc", 3, HeightfieldShapeDesc, ShapeDesc)

void HeightfieldShapeDesc::setHeightfield(const resource::Id< hf::Heightfield >& heightfield)
{
	m_heightfield = heightfield;
}

const resource::Id< hf::Heightfield >& HeightfieldShapeDesc::getHeightfield() const
{
	return m_heightfield;
}

void HeightfieldShapeDesc::setMaterialMask(const resource::Id< hf::MaterialMask >& materialMask)
{
	m_materialMask = materialMask;
}

const resource::Id< hf::MaterialMask >& HeightfieldShapeDesc::getMaterialMask() const
{
	return m_materialMask;
}

void HeightfieldShapeDesc::serialize(ISerializer& s)
{
	ShapeDesc::serialize(s);

	s >> resource::Member< hf::Heightfield >(L"heightfield", m_heightfield);

	if (s.getVersion() >= 2)
		s >> resource::Member< hf::MaterialMask >(L"materialMask", m_materialMask);
}

	}
}
