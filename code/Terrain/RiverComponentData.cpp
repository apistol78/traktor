#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/RiverComponentData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.RiverComponentData", 0, RiverComponentData, world::IEntityComponentData)

RiverComponentData::RiverComponentData()
:	m_tileFactorV(1.0f)
{
}

void RiverComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberAlignedVector< ControlPoint, MemberComposite< ControlPoint > >(L"path", m_path);
	s >> Member< float >(L"tileFactorV", m_tileFactorV, AttributeRange(0.0f));
}

RiverComponentData::ControlPoint::ControlPoint()
:	position(Vector4::zero())
,	width(0.0f)
,	tension(1.0f)
{
}

void RiverComponentData::ControlPoint::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"position", position);
	s >> Member< float >(L"width", width);
	s >> Member< float >(L"tension", tension, AttributeRange(0.1f, 10.0f));
}

	}
}
