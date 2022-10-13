#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/RiverComponentData.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.RiverComponentData", 0, RiverComponentData, world::IEntityComponentData)

void RiverComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void RiverComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberAlignedVector< ControlPoint, MemberComposite< ControlPoint > >(L"path", m_path);
	s >> Member< float >(L"tileFactorV", m_tileFactorV, AttributeRange(0.0f));
}

void RiverComponentData::ControlPoint::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"position", position);
	s >> Member< float >(L"width", width);
	s >> Member< float >(L"tension", tension, AttributeRange(0.1f, 10.0f));
}

}
