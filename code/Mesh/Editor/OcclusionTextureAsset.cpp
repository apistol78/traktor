
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Mesh/Editor/OcclusionTextureAsset.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.OcclusionTextureAsset", 13, OcclusionTextureAsset, editor::Asset)

void OcclusionTextureAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);
	s >> MemberComposite< render::TextureOutput, false >(L"output", m_output);
}

	}
}
