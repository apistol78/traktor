#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeAngles.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
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

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.LightComponentData", 7, LightComponentData, IEntityComponentData)

LightComponentData::LightComponentData()
:	m_lightType(LtDisabled)
,	m_color(1.0f, 1.0f, 1.0f, 0.0f)
,	m_intensity(1000.0f)
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

	if (s.getVersion() >= 5)
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

	if (s.getVersion() >= 7)
		s >> Member< float >(L"intensity", m_intensity, AttributeRange(0.0f) | AttributeUnit(AuLumens));

	if (s.getVersion() >= 1)
	{
		if (s.getVersion() >= 3)
		{
			s >> resource::Member< render::ITexture >(L"probeDiffuseTexture", m_probeDiffuseTexture);
			if (s.getVersion() >= 4)
				s >> resource::Member< render::ITexture >(L"probeSpecularTexture", m_probeSpecularTexture);
		}
		else if (s.getVersion() >= 2)
		{
			s >> resource::Member< render::ITexture >(L"probeDiffuseTexture", m_probeDiffuseTexture);

			resource::Id< render::ITexture > probeSpecularTexture;
			s >> resource::Member< render::ITexture >(L"probeSpecularTexture", probeSpecularTexture);
		}
		else
			s >> resource::Member< render::ITexture >(L"probeTexture", m_probeDiffuseTexture);
	}

	if (s.getVersion() < 6)
	{
		resource::Id< render::ITexture > cloudShadowTexture;
		s >> resource::Member< render::ITexture >(L"cloudShadowTexture", cloudShadowTexture);
	}

	s >> Member< bool >(L"castShadow", m_castShadow);
	s >> Member< float >(L"range", m_range, AttributeUnit(AuMetres));
	s >> Member< float >(L"radius", m_radius, AttributeUnit(AuRadians) | AttributeAngles());
	s >> Member< float >(L"flickerAmount", m_flickerAmount);
	s >> Member< float >(L"flickerFilter", m_flickerFilter);
}

	}
}
