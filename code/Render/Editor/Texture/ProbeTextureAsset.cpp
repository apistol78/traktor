/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeAngles.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Texture/ProbeTextureAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ProbeTextureAsset", 1, ProbeTextureAsset, editor::Asset)

ProbeTextureAsset::ProbeTextureAsset()
:	m_glossScale(10)
,	m_glossBias(1)
,	m_sizeDivisor(1)
{
}

void ProbeTextureAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion< ProbeTextureAsset >() >= 1)
	{
		s >> Member< int32_t >(L"glossScale", m_glossScale);
		s >> Member< int32_t >(L"glossBias", m_glossBias);
		s >> Member< int32_t >(L"sizeDivisor", m_sizeDivisor);
	}
	else
	{
		float filterAngle;
		s >> Member< float >(L"filterAngle", filterAngle);
	}
}

	}
}
