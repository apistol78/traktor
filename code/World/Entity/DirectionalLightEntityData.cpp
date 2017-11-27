/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/ITexture.h"
#include "Resource/Member.h"
#include "World/Entity/DirectionalLightEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.DirectionalLightEntityData", 2, DirectionalLightEntityData, EntityData)

DirectionalLightEntityData::DirectionalLightEntityData()
:	m_sunColor(1.0f, 1.0f, 1.0f, 0.0f)
,	m_baseColor(0.5f, 0.5f, 0.5f, 0.0f)
,	m_shadowColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_castShadow(true)
{
}

void DirectionalLightEntityData::serialize(ISerializer& s)
{
	EntityData::serialize(s);

	s >> Member< Vector4 >(L"sunColor", m_sunColor);
	s >> Member< Vector4 >(L"baseColor", m_baseColor);
	s >> Member< Vector4 >(L"shadowColor", m_shadowColor);

	if (s.getVersion() >= 2)
		s >> resource::Member< render::ITexture >(L"cloudShadowTexture", m_cloudShadowTexture);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"castShadow", m_castShadow);
}

	}
}
