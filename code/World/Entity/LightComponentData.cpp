/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Render/ITexture.h"
#include "Resource/Member.h"
#include "World/Entity/LightComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.LightComponentData", 2, LightComponentData, IEntityComponentData)

LightComponentData::LightComponentData()
:	m_lightType(LtDisabled)
,	m_sunColor(1.0f, 1.0f, 1.0f, 0.0f)
,	m_baseColor(0.5f, 0.5f, 0.5f, 0.0f)
,	m_shadowColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_castShadow(false)
,	m_range(10.0f)
,	m_radius(HALF_PI)
,	m_flickerAmount(0.0f)
,	m_flickerFilter(0.0f)
{
}

void LightComponentData::serialize(ISerializer& s)
{
	const MemberEnum< LightType >::Key c_LightType_Keys[] =
	{
		{ L"LtDisabled", LtDisabled },
		{ L"LtDirectional", LtDirectional },
		{ L"LtPoint", LtPoint },
		{ L"LtSpot", LtSpot },
		{ L"LtProbe", LtProbe },
		{ 0 }
	};

	s >> MemberEnum< LightType >(L"lightType", m_lightType, c_LightType_Keys);
	s >> Member< Vector4 >(L"sunColor", m_sunColor);
	s >> Member< Vector4 >(L"baseColor", m_baseColor);
	s >> Member< Vector4 >(L"shadowColor", m_shadowColor);

	if (s.getVersion< LightComponentData >() >= 1)
	{
		if (s.getVersion< LightComponentData >() >= 2)
		{
			s >> resource::Member< render::ITexture >(L"probeDiffuseTexture", m_probeDiffuseTexture);
			s >> resource::Member< render::ITexture >(L"probeSpecularTexture", m_probeSpecularTexture);
		}
		else
			s >> resource::Member< render::ITexture >(L"probeTexture", m_probeDiffuseTexture);
	}

	s >> resource::Member< render::ITexture >(L"cloudShadowTexture", m_cloudShadowTexture);
	s >> Member< bool >(L"castShadow", m_castShadow);
	s >> Member< float >(L"range", m_range);
	s >> Member< float >(L"radius", m_radius);
	s >> Member< float >(L"flickerAmount", m_flickerAmount);
	s >> Member< float >(L"flickerFilter", m_flickerFilter);
}

	}
}
