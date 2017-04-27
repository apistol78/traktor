/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/OcclusionTextureAsset.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.OcclusionTextureAsset", 2, OcclusionTextureAsset, ISerializable)

OcclusionTextureAsset::OcclusionTextureAsset()
:	m_size(1024)
,	m_traceDistance(16.0f)
,	m_blurRadius(0)
{
}

void OcclusionTextureAsset::serialize(ISerializer& s)
{
	s >> resource::Member< Heightfield >(L"heightfield", m_heightfield);
	s >> resource::Member< ISerializable >(L"occluderData", m_occluderData);

	if (s.getVersion() >= 1)
	{
		s >> Member< uint32_t >(L"size", m_size, AttributeRange(16));
		s >> Member< float >(L"traceDistance", m_traceDistance, AttributeRange(0.0f));
	}

	if (s.getVersion() >= 2)
		s >> Member< int32_t >(L"blurRadius", m_blurRadius, AttributeRange(0));
}

	}
}
