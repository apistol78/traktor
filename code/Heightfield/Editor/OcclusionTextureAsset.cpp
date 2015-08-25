#include "Core/Serialization/ISerializer.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/OcclusionTextureAsset.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.OcclusionTextureAsset", 1, OcclusionTextureAsset, ISerializable)

OcclusionTextureAsset::OcclusionTextureAsset()
:	m_size(1024)
,	m_traceDistance(16.0f)
{
}

void OcclusionTextureAsset::serialize(ISerializer& s)
{
	s >> resource::Member< Heightfield >(L"heightfield", m_heightfield);
	s >> resource::Member< ISerializable >(L"occluderData", m_occluderData);

	if (s.getVersion() >= 1)
	{
		s >> Member< uint32_t >(L"size", m_size);
		s >> Member< float >(L"traceDistance", m_traceDistance);
	}
}

	}
}
