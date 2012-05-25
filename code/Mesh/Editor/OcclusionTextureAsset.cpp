
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Mesh/Editor/OcclusionTextureAsset.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.OcclusionTextureAsset", 5, OcclusionTextureAsset, editor::Asset)

bool OcclusionTextureAsset::serialize(ISerializer& s)
{
	if (!editor::Asset::serialize(s))
		return false;

	s >> MemberComposite< render::TextureOutput, false >(L"output", m_output);
	return true;
}

	}
}
