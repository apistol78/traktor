#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Shader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Resource/Member.h"
#include "Terrain/RiverEntityData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.RiverEntityData", 0, RiverEntityData, world::EntityData)

bool RiverEntityData::serialize(ISerializer& s)
{
	if (!world::EntityData::serialize(s))
		return false;

	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	s >> MemberAlignedVector< ControlPoint, MemberComposite< ControlPoint > >(L"path", m_path);

	return true;
}

RiverEntityData::ControlPoint::ControlPoint()
:	width(0.0f)
{
}

bool RiverEntityData::ControlPoint::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"position", position);
	s >> Member< float >(L"width", width);
	return true;
}

	}
}
