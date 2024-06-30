/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeHdr.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/ITexture.h"
#include "Render/Image2/ImageGraph.h"
#include "Resource/Member.h"
#include "World/IrradianceGrid.h"
#include "World/WorldRenderSettings.h"

namespace traktor::world
{
	namespace
	{

const wchar_t* c_ShadowSettings_elementNames18[] =
{
	L"disabled",
	L"low",
	L"medium",
	L"high",
	L"highest"
};

const wchar_t* c_ShadowSettings_elementNames[] =
{
	L"disabled",
	L"low",
	L"medium",
	L"high",
	L"ultra"
};

const wchar_t* c_ImageProcess_elementNames[] =
{
	L"disabled",
	L"low",
	L"medium",
	L"high",
	L"ultra"
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRenderSettings", 40, WorldRenderSettings, ISerializable)

void WorldRenderSettings::serialize(ISerializer& s)
{
	T_ASSERT(s.getVersion() >= 17);

	s >> Member< float >(L"viewNearZ", viewNearZ, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"viewFarZ", viewFarZ, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));

	if (s.getVersion() < 35)
		s >> ObsoleteMember< bool >(L"linearLighting");

	if (s.getVersion() >= 28)
	{
		if (s.getVersion() >= 33)
		{
			if (s.getVersion() >= 38)
			{
				const MemberEnum< ExposureMode >::Key c_ExposureMode_Keys[] =
				{
					{ L"Fixed", Fixed },
					{ L"Adaptive", Adaptive },
					{ 0 }
				};
				s >> MemberEnum< ExposureMode >(L"exposureMode", exposureMode, c_ExposureMode_Keys);
			}
			else
			{
				const MemberEnum< ExposureMode >::Key c_ExposureMode_Keys[] =
				{
					{ L"EmFixed", Fixed },
					{ L"EmAdaptive", Adaptive },
					{ 0 }
				};
				s >> MemberEnum< ExposureMode >(L"exposureMode", exposureMode, c_ExposureMode_Keys);
			}
			s >> Member< float >(L"exposure", exposure, AttributeUnit(UnitType::EV));
		}
		else
		{
			exposureMode = Adaptive;
			s >> Member< float >(L"exposureBias", exposure, AttributeRange(0.0f));
		}
	}

	if (s.getVersion() >= 23)
	{
		if (s.getVersion() < 25)
			s >> ObsoleteMember< bool >(L"occlusionCulling");

		if (s.getVersion() < 29)
			s >> ObsoleteMember< bool >(L"depthPass");
	}
	else
	{
		if (s.getVersion() < 25)
			s >> ObsoleteMember< bool >(L"occlusionCullingEnabled");

		s >> ObsoleteMember< bool >(L"depthPassEnabled");
	}

	if (s.getVersion() >= 19)
		s >> MemberStaticArray< ShadowSettings, sizeof_array(shadowSettings), MemberComposite< ShadowSettings > >(L"shadowSettings", shadowSettings, c_ShadowSettings_elementNames);
	else
		s >> MemberStaticArray< ShadowSettings, sizeof_array(shadowSettings), MemberComposite< ShadowSettings > >(L"shadowSettings", shadowSettings, c_ShadowSettings_elementNames18);

	if (s.getVersion() >= 26 && s.getVersion() < 30)
		s >> ObsoleteMember< Color4f >(L"ambientColor", AttributeHdr());
	
	if (s.getVersion() >= 23)
	{
		if (s.getVersion() < 24)
		{
			s >> ObsoleteMember< bool >(L"motionBlur");
			s >> ObsoleteMember< float >(L"motionBlurAmount");
		}
		if (s.getVersion() < 36)
			s >> ObsoleteMember< bool >(L"fog");
	}
	else
	{
		if (s.getVersion() < 36)
			s >> ObsoleteMember< bool >(L"fogEnabled");
	}

	if (s.getVersion() >= 21)
	{
		if (s.getVersion() >= 34)
		{
			s >> Member< float >(L"fogDistance", fogDistance, AttributeUnit(UnitType::Metres));
			s >> Member< float >(L"fogDensity", fogDensity, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
			if (s.getVersion() >= 39)
				s >> Member< float >(L"fogDensityMax", fogDensityMax, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
		}
		else
		{
			s >> ObsoleteMember< float >(L"fogDistanceY", AttributeUnit(UnitType::Metres));
			s >> Member< float >(L"fogDistanceZ", fogDistance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
			s >> ObsoleteMember< float >(L"fogDensityY", AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Metres));
			s >> Member< float >(L"fogDensityZ", fogDensity, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Metres));
		}
	}

	if (s.getVersion() >= 26)
		s >> Member< Color4f >(L"fogColor", fogColor, AttributeHdr());
	else
	{
		Color4ub fc;
		s >> Member< Color4ub >(L"fogColor", fc);
		fogColor = Color4f(fc.r / 255.0f, fc.g / 255.0f, fc.b / 255.0f, fc.a / 255.0f);
	}

	if (s.getVersion() >= 20 && s.getVersion() <= 30)
		s >> resource::ObsoleteMember< render::ITexture >(L"reflectionMap");

	if (s.getVersion() >= 32 && s.getVersion() < 40)
		s >> resource::ObsoleteMember< IrradianceGrid >(L"irradianceGrid");

	if (s.getVersion() >= 22)
		s >> MemberStaticArray< resource::Id< render::ImageGraph >, sizeof_array(imageProcess), resource::Member< render::ImageGraph > >(L"imageProcess", imageProcess, c_ImageProcess_elementNames);

	if (s.getVersion() >= 37)
		s >> resource::Member< render::ITexture >(L"colorGrading", colorGrading);
}

void WorldRenderSettings::ShadowSettings::serialize(ISerializer& s)
{
	if (s.getVersion() >= 18 && s.getVersion() < 38)
	{
		enum ShadowProjection
		{
			Unused
		};
		const MemberEnum< ShadowProjection >::Key c_ShadowProjection_Keys[] =
		{
			{ L"SpBox", Unused },
			{ L"SpLiSP", Unused },
			{ L"SpTrapezoid", Unused },
			{ L"SpUniform", Unused },
			{ 0 }
		};
		ShadowProjection projection;
		s >> MemberEnum< ShadowProjection >(L"projection", projection, c_ShadowProjection_Keys);
	}

	s >> Member< float >(L"farZ", farZ, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< int32_t >(L"resolution", resolution, AttributeRange(1));
	s >> Member< float >(L"bias", bias, AttributeRange(0.0f, 8.0f));

	if (s.getVersion() < 38)
	{
		float biasCoeff;
		s >> Member< float >(L"biasCoeff", biasCoeff);
	}

	s >> Member< int32_t >(L"cascadingSlices", cascadingSlices, AttributeRange(1, MaxSliceCount));
	s >> Member< float >(L"cascadingLambda", cascadingLambda, AttributeRange(0.0f, 10.0f));
	s >> Member< bool >(L"quantizeProjection", quantizeProjection);
	s >> Member< int32_t >(L"maskDenominator", maskDenominator, AttributeRange(1));
	s >> resource::Member< render::ImageGraph >(L"maskProject", maskProject);

	if (s.getVersion() < 27)
	{
		resource::Id< render::ImageGraph > maskFilter;
		s >> resource::Member< render::ImageGraph >(L"maskFilter", maskFilter);
	}
}

}
