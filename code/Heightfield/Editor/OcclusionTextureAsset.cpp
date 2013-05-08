#include "Core/Serialization/ISerializer.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/OcclusionTextureAsset.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.OcclusionTextureAsset", 0, OcclusionTextureAsset, ISerializable)

void OcclusionTextureAsset::serialize(ISerializer& s)
{
	s >> resource::Member< Heightfield >(L"heightfield", m_heightfield);
	s >> resource::Member< ISerializable >(L"occluderData", m_occluderData);
}

	}
}
