/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/AttributeAngles.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Texture/ProbeTextureAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ProbeTextureAsset", 0, ProbeTextureAsset, editor::Asset)

ProbeTextureAsset::ProbeTextureAsset()
:	m_filterAngle(deg2rad(20.0f))
{
}

void ProbeTextureAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	s >> Member< float >(L"filterAngle", m_filterAngle, AttributeAngles() | AttributeRange(0.0f, HALF_PI));
}

	}
}
