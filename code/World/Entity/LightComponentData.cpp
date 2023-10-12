/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"
#include "Render/ITexture.h"
#include "Render/SH/SHCoeffs.h"
#include "Resource/Member.h"
#include "World/Entity/LightComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.LightComponentData", 12, LightComponentData, IEntityComponentData)

int32_t LightComponentData::getOrdinal() const
{
	return 0;
}

void LightComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void LightComponentData::serialize(ISerializer& s)
{
	if (s.getVersion< LightComponentData >() >= 11)
	{
		const MemberEnum< LightType >::Key c_LightType_Keys[] =
		{
			{ L"Disabled", LightType::Disabled },
			{ L"Directional", LightType::Directional },
			{ L"Point", LightType::Point },
			{ L"Spot", LightType::Spot },
			{ 0 }
		};
		s >> MemberEnum< LightType >(L"lightType", m_lightType, c_LightType_Keys);
	}
	else
	{
		const MemberEnum< LightType >::Key c_LightType_Keys[] =
		{
			{ L"LtDisabled", LightType::Disabled },
			{ L"LtDirectional", LightType::Directional },
			{ L"LtPoint", LightType::Point },
			{ L"LtSpot", LightType::Spot },
			{ 0 }
		};
		s >> MemberEnum< LightType >(L"lightType", m_lightType, c_LightType_Keys);
	}

	if (s.getVersion< LightComponentData >() >= 5)
	{
		s >> Member< Color4f >(L"color", m_color);
	}
	else
	{
		Vector4 sunColor = Vector4::zero();
		Vector4 baseColor = Vector4::zero();
		Vector4 shadowColor = Vector4::zero();

		s >> Member< Vector4 >(L"sunColor", sunColor);
		s >> Member< Vector4 >(L"baseColor", baseColor);
		s >> Member< Vector4 >(L"shadowColor", shadowColor);

		m_color = Color4f(sunColor);
	}

	if (s.getVersion< LightComponentData >() >= 7)
		s >> Member< float >(L"intensity", m_intensity, AttributeRange(0.0f) | AttributeUnit(UnitType::Lumens));

	if (s.getVersion< LightComponentData >() >= 1 && s.getVersion< LightComponentData >() < 8)
	{
		resource::Id< render::ITexture > probeDiffuseTexture;
		resource::Id< render::ITexture > probeSpecularTexture;
		if (s.getVersion< LightComponentData >() >= 3)
		{
			s >> resource::Member< render::ITexture >(L"probeDiffuseTexture", probeDiffuseTexture);
			if (s.getVersion< LightComponentData >() >= 4)
				s >> resource::Member< render::ITexture >(L"probeSpecularTexture", probeSpecularTexture);
		}
		else if (s.getVersion< LightComponentData >() >= 2)
		{
			s >> resource::Member< render::ITexture >(L"probeDiffuseTexture", probeDiffuseTexture);
			s >> resource::Member< render::ITexture >(L"probeSpecularTexture", probeSpecularTexture);
		}
		else
			s >> resource::Member< render::ITexture >(L"probeTexture", probeDiffuseTexture);
	}

	if (s.getVersion< LightComponentData >() < 6)
	{
		resource::Id< render::ITexture > cloudShadowTexture;
		s >> resource::Member< render::ITexture >(L"cloudShadowTexture", cloudShadowTexture);
	}

	s >> Member< bool >(L"castShadow", m_castShadow);

	if (s.getVersion< LightComponentData >() >= 12)
	{
		s >> Member< float >(L"nearRange", m_nearRange, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
		s >> Member< float >(L"farRange", m_farRange, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	}
	else
	{
		m_nearRange = 0.0f;
		s >> Member< float >(L"range", m_farRange, AttributeUnit(UnitType::Metres));
	}

	s >> Member< float >(L"radius", m_radius, AttributeUnit(UnitType::Radians));
	s >> Member< float >(L"flickerAmount", m_flickerAmount, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
	s >> Member< float >(L"flickerFilter", m_flickerFilter, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));

	if (s.getVersion< LightComponentData >() >= 8 && s.getVersion< LightComponentData >() < 9)
	{
		Ref< const render::SHCoeffs > shCoeffs;
		s >> MemberRef< const render::SHCoeffs >(L"shCoeffs", shCoeffs);
	}

	if (s.getVersion< LightComponentData >() >= 10)
	{
		const MemberEnum< LightBakeMode >::Key c_LightBakeMode_Keys[] =
		{
			{ L"LbmDisabled", LbmDisabled },
			{ L"LbmIndirect", LbmIndirect },
			{ L"LbmAll", LbmAll },
			{ 0 }
		};
		s >> MemberEnum< LightBakeMode >(L"bakeMode", m_bakeMode, c_LightBakeMode_Keys);
	}
}

}
