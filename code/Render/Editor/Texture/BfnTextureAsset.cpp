/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Texture/BfnTextureAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.BfnTextureAsset", 1, BfnTextureAsset, ISerializable)

BfnTextureAsset::BfnTextureAsset()
:	m_bestFitFactorOnly(true)
,	m_collapseSymmetry(true)
,	m_size(512)
{
}

void BfnTextureAsset::serialize(ISerializer& s)
{
	s >> Member< bool >(L"bestFitFactorOnly", m_bestFitFactorOnly);
	if (s.getVersion< BfnTextureAsset >() >= 1)
	{
		s >> Member< bool >(L"collapseSymmetry", m_collapseSymmetry);
		s >> Member< int32_t >(L"size", m_size);
	}
}

	}
}
