/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/ITexture.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Resource/Member.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{
	namespace world
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRenderSettings", 25, WorldRenderSettings, ISerializable)

WorldRenderSettings::WorldRenderSettings()
:	viewNearZ(1.0f)
,	viewFarZ(100.0f)
,	linearLighting(true)
,	depthPass(true)
,	fog(false)
,	fogDistanceY(0.0f)
,	fogDistanceZ(90.0f)
,	fogDensityY(0.0f)
,	fogDensityZ(0.0f)
,	fogColor(255, 255, 255, 255)
{
}

void WorldRenderSettings::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 17);

	s >> Member< float >(L"viewNearZ", viewNearZ, AttributeRange(0.0f));
	s >> Member< float >(L"viewFarZ", viewFarZ, AttributeRange(0.0f));
	s >> Member< bool >(L"linearLighting", linearLighting);

	if (s.getVersion() >= 23)
	{
		if (s.getVersion() < 25)
		{
			bool dummy = false;
			s >> Member< bool >(L"occlusionCulling", dummy);
		}
		s >> Member< bool >(L"depthPass", depthPass);
	}
	else
	{
		if (s.getVersion() < 25)
		{
			bool dummy = false;
			s >> Member< bool >(L"occlusionCulling", dummy);
		}
		s >> Member< bool >(L"depthPassEnabled", depthPass);
	}

	if (s.getVersion() >= 19)
		s >> MemberStaticArray< ShadowSettings, sizeof_array(shadowSettings), MemberComposite< ShadowSettings > >(L"shadowSettings", shadowSettings, c_ShadowSettings_elementNames);
	else
		s >> MemberStaticArray< ShadowSettings, sizeof_array(shadowSettings), MemberComposite< ShadowSettings > >(L"shadowSettings", shadowSettings, c_ShadowSettings_elementNames18);

	if (s.getVersion() >= 23)
	{
		if (s.getVersion() < 24)
		{
			float motionBlurAmount = 0.0f;
			bool motionBlur = false;

			s >> Member< bool >(L"motionBlur", motionBlur);
			s >> Member< float >(L"motionBlurAmount", motionBlurAmount);
		}
		s >> Member< bool >(L"fog", fog);
	}
	else
	{
		s >> Member< bool >(L"fogEnabled", fog);
	}

	if (s.getVersion() >= 21)
	{
		s >> Member< float >(L"fogDistanceY", fogDistanceY);
		s >> Member< float >(L"fogDistanceZ", fogDistanceZ, AttributeRange(0.0f));
		s >> Member< float >(L"fogDensityY", fogDensityY, AttributeRange(0.0f, 1.0f));
		s >> Member< float >(L"fogDensityZ", fogDensityZ, AttributeRange(0.0f, 1.0f));
	}

	s >> Member< Color4ub >(L"fogColor", fogColor);

	if (s.getVersion() >= 20)
		s >> resource::Member< render::ITexture >(L"reflectionMap", reflectionMap);

	if (s.getVersion() >= 22)
		s >> MemberStaticArray< resource::Id< render::ImageProcessSettings >, sizeof_array(imageProcess), resource::Member< render::ImageProcessSettings > >(L"imageProcess", imageProcess, c_ImageProcess_elementNames);
}

WorldRenderSettings::ShadowSettings::ShadowSettings()
:	projection(SpUniform)
,	farZ(0.0f)
,	resolution(1024)
,	bias(0.0f)
,	biasCoeff(1.0f)
,	cascadingSlices(1)
,	cascadingLambda(0.0f)
,	quantizeProjection(false)
,	maskDenominator(1)
{
}

void WorldRenderSettings::ShadowSettings::serialize(ISerializer& s)
{
	const MemberEnum< ShadowProjection >::Key c_ShadowProjection_Keys[] =
	{
		{ L"SpBox", SpBox },
		{ L"SpLiSP", SpLiSP },
		{ L"SpTrapezoid", SpTrapezoid },
		{ L"SpUniform", SpUniform },
		{ 0 }
	};

	if (s.getVersion() >= 18)
		s >> MemberEnum< ShadowProjection >(L"projection", projection, c_ShadowProjection_Keys);

	s >> Member< float >(L"farZ", farZ, AttributeRange(0.0f));
	s >> Member< int32_t >(L"resolution", resolution, AttributeRange(1));
	s >> Member< float >(L"bias", bias);
	s >> Member< float >(L"biasCoeff", biasCoeff);
	s >> Member< int32_t >(L"cascadingSlices", cascadingSlices, AttributeRange(1, MaxSliceCount));
	s >> Member< float >(L"cascadingLambda", cascadingLambda, AttributeRange(0.0f));
	s >> Member< bool >(L"quantizeProjection", quantizeProjection);
	s >> Member< int32_t >(L"maskDenominator", maskDenominator, AttributeRange(1));
	s >> resource::Member< render::ImageProcessSettings >(L"maskProject", maskProject);
	s >> resource::Member< render::ImageProcessSettings >(L"maskFilter", maskFilter);
}

	}
}
