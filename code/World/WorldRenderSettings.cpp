#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/ITexture.h"
#include "Resource/Member.h"
#include "World/WorldRenderSettings.h"
#include "World/PostProcess/PostProcessSettings.h"

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

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRenderSettings", 20, WorldRenderSettings, ISerializable)

WorldRenderSettings::WorldRenderSettings()
:	viewNearZ(1.0f)
,	viewFarZ(100.0f)
,	linearLighting(true)
,	occlusionCullingEnabled(false)
,	depthPassEnabled(true)
,	fogEnabled(false)
,	fogDistance(90.0f)
,	fogRange(10.0f)
,	fogColor(255, 255, 255, 255)
{
}

bool WorldRenderSettings::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 17);

	s >> Member< float >(L"viewNearZ", viewNearZ, AttributeRange(0.0f));
	s >> Member< float >(L"viewFarZ", viewFarZ, AttributeRange(0.0f));
	s >> Member< bool >(L"linearLighting", linearLighting);
	s >> Member< bool >(L"occlusionCullingEnabled", occlusionCullingEnabled);
	s >> Member< bool >(L"depthPassEnabled", depthPassEnabled);
	if (s.getVersion() >= 19)
		s >> MemberStaticArray< ShadowSettings, sizeof_array(shadowSettings), MemberComposite< ShadowSettings > >(L"shadowSettings", shadowSettings, c_ShadowSettings_elementNames);
	else
		s >> MemberStaticArray< ShadowSettings, sizeof_array(shadowSettings), MemberComposite< ShadowSettings > >(L"shadowSettings", shadowSettings, c_ShadowSettings_elementNames18);
	s >> Member< bool >(L"fogEnabled", fogEnabled);
	s >> Member< float >(L"fogDistance", fogDistance, AttributeRange(0.0f));
	s >> Member< float >(L"fogRange", fogRange, AttributeRange(0.0f));
	s >> Member< Color4ub >(L"fogColor", fogColor);
	if (s.getVersion() >= 20)
		s >> resource::Member< render::ITexture >(L"reflectionMap", reflectionMap);

	return true;
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

bool WorldRenderSettings::ShadowSettings::serialize(ISerializer& s)
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
	s >> resource::Member< PostProcessSettings >(L"maskProject", maskProject);
	s >> resource::Member< PostProcessSettings >(L"maskFilter", maskFilter);

	return true;
}

	}
}
