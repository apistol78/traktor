#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/RiverEntityData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.RiverEntityData", 2, RiverEntityData, world::EntityData)

RiverEntityData::RiverEntityData()
:	m_tileFactorV(1.0f)
{
}

void RiverEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);

	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberAlignedVector< ControlPoint, MemberComposite< ControlPoint > >(L"path", m_path);

	if (s.getVersion() >= 2)
		s >> Member< float >(L"tileFactorV", m_tileFactorV, AttributeRange(0.0f));
}

RiverEntityData::ControlPoint::ControlPoint()
:	position(Vector4::zero())
,	width(0.0f)
,	tension(1.0f)
{
}

void RiverEntityData::ControlPoint::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"position", position);
	s >> Member< float >(L"width", width);

	if (s.getVersion() >= 1)
		s >> Member< float >(L"tension", tension, AttributeRange(0.1f, 10.0f));
}

	}
}
