#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/Member.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldTypes.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRenderSettings", 16, WorldRenderSettings, ISerializable)

WorldRenderSettings::WorldRenderSettings()
:	renderType(RtForward)
,	viewNearZ(1.0f)
,	viewFarZ(100.0f)
,	linearLighting(true)
,	occlusionCullingEnabled(false)
,	depthPassEnabled(true)
,	shadowsEnabled(false)
,	shadowsProjection(SpUniform)
,	shadowsQuality(SqMedium)
,	shadowFarZ(100.0f)
,	shadowMapResolution(1024)
,	shadowMapBias(0.001f)
,	shadowMapBiasCoeff(0.1f)
,	shadowCascadingSlices(1)
,	shadowCascadingLambda(1.0f)
,	shadowQuantizeProjection(true)
,	ambientOcclusionQuality(AoqHigh)
,	fogEnabled(false)
,	fogDistance(90.0f)
,	fogRange(10.0f)
,	fogColor(255, 255, 255, 255)
,	antiAliasQuality(AaqMedium)
{
}

WorldRenderSettings::WorldRenderSettings(const WorldRenderSettings& settings)
:	renderType(settings.renderType)
,	viewNearZ(settings.viewNearZ)
,	viewFarZ(settings.viewFarZ)
,	linearLighting(settings.linearLighting)
,	occlusionCullingEnabled(settings.occlusionCullingEnabled)
,	depthPassEnabled(settings.depthPassEnabled)
,	shadowsEnabled(settings.shadowsEnabled)
,	shadowsProjection(settings.shadowsProjection)
,	shadowsQuality(settings.shadowsQuality)
,	shadowFarZ(settings.shadowFarZ)
,	shadowMapResolution(settings.shadowMapResolution)
,	shadowMapBias(settings.shadowMapBias)
,	shadowMapBiasCoeff(settings.shadowMapBiasCoeff)
,	shadowCascadingSlices(settings.shadowCascadingSlices)
,	shadowCascadingLambda(settings.shadowCascadingLambda)
,	shadowQuantizeProjection(settings.shadowQuantizeProjection)
,	ambientOcclusionQuality(settings.ambientOcclusionQuality)
,	fogEnabled(settings.fogEnabled)
,	fogDistance(settings.fogDistance)
,	fogRange(settings.fogRange)
,	fogColor(settings.fogColor)
,	antiAliasQuality(settings.antiAliasQuality)
{
}

bool WorldRenderSettings::serialize(ISerializer& s)
{
	const MemberEnum< RenderType >::Key c_RenderType_Keys[] =
	{
		{ L"RtForward", RtForward },
		{ L"RtPreLit", RtPreLit },
		{ 0 }
	};

	const MemberEnum< ShadowProjection >::Key c_ShadowProjection_Keys[] =
	{
		{ L"SpBox", SpBox },
		{ L"SpLiSP", SpLiSP },
		{ L"SpTrapezoid", SpTrapezoid },
		{ L"SpUniform", SpUniform },
		{ 0 }
	};

	const MemberEnum< ShadowQuality >::Key c_ShadowQuality_Keys[] =
	{
		{ L"SqNoFilter", SqNoFilter },
		{ L"SqLow", SqLow },
		{ L"SqMedium", SqMedium },
		{ L"SqHigh", SqHigh },
		{ L"SqHighest", SqHighest },
		{ 0 }
	};

	const MemberEnum< AmbientOcclusionQuality >::Key c_AmbientOcclusionQuality_Keys[] =
	{
		{ L"AoqDisabled", AoqDisabled },
		{ L"AoqLow", AoqLow },
		{ L"AoqMedium", AoqMedium },
		{ L"AoqHigh", AoqHigh },
		{ L"AoqHighest", AoqHighest },
		{ 0 }
	};

	const MemberEnum< AntiAliasQuality >::Key c_AntiAliasQuality_Keys[] =
	{
		{ L"AaqDisabled", AaqDisabled },
		{ L"AaqLow", AaqLow },
		{ L"AaqMedium", AaqMedium },
		{ L"AaqHigh", AaqHigh },
		{ L"AaqHighest", AaqHighest },
		{ 0 }
	};

	if (s.getVersion() >= 8)
		s >> MemberEnum< RenderType >(L"renderType", renderType, c_RenderType_Keys);

	s >> Member< float >(L"viewNearZ", viewNearZ, AttributeRange(0.0f));
	s >> Member< float >(L"viewFarZ", viewFarZ, AttributeRange(0.0f));

	if (s.getVersion() >= 16)
		s >> Member< bool >(L"linearLighting", linearLighting);

	if (s.getVersion() >= 13)
		s >> Member< bool >(L"occlusionCullingEnabled", occlusionCullingEnabled);

	s >> Member< bool >(L"depthPassEnabled", depthPassEnabled);

	if (s.getVersion() >= 6 && s.getVersion() < 10)
	{
		float depthRange = 0.0f;
		s >> Member< float >(L"depthRange", depthRange);
	}
	
	if (s.getVersion() >= 1 && s.getVersion() < 11)
	{
		bool velocityPassEnable = false;
		s >> Member< bool >(L"velocityPassEnable", velocityPassEnable);
	}

	s >> Member< bool >(L"shadowsEnabled", shadowsEnabled);

	if (s.getVersion() >= 5)
		s >> MemberEnum< ShadowProjection >(L"shadowsProjection", shadowsProjection, c_ShadowProjection_Keys);

	if (s.getVersion() >= 4)
		s >> MemberEnum< ShadowQuality >(L"shadowsQuality", shadowsQuality, c_ShadowQuality_Keys);

	if (s.getVersion() >= 3 && s.getVersion() < 10)
	{
		bool ssaoEnabled = false;
		s >> Member< bool >(L"ssaoEnabled", ssaoEnabled);
	}

	if (s.getVersion() <= 1)
	{
		int32_t shadowCascadingSlices; float shadowCascadingLambda;
		s >> Member< int32_t >(L"shadowCascadingSlices", shadowCascadingSlices);
		s >> Member< float >(L"shadowCascadingLambda", shadowCascadingLambda, AttributeRange(0.0f, 1.0f));
	}

	s >> Member< float >(L"shadowFarZ", shadowFarZ, AttributeRange(0.0f));
	s >> Member< int32_t >(L"shadowMapResolution", shadowMapResolution);
	s >> Member< float >(L"shadowMapBias", shadowMapBias);

	if (s.getVersion() >= 12)
		s >> Member< float >(L"shadowMapBiasCoeff", shadowMapBiasCoeff);

	if (s.getVersion() >= 7)
	{
		s >> Member< int32_t >(L"shadowCascadingSlices", shadowCascadingSlices, AttributeRange(1, MaxSliceCount));
		s >> Member< float >(L"shadowCascadingLambda", shadowCascadingLambda, AttributeRange(0.0f));
	}

	if (s.getVersion() >= 11)
		s >> Member< bool >(L"shadowQuantizeProjection", shadowQuantizeProjection);

	if (s.getVersion() >= 14)
		s >> MemberEnum< AmbientOcclusionQuality >(L"ambientOcclusionQuality", ambientOcclusionQuality, c_AmbientOcclusionQuality_Keys);

	if (s.getVersion() >= 9)
	{
		s >> Member< bool >(L"fogEnabled", fogEnabled);
		s >> Member< float >(L"fogDistance", fogDistance, AttributeRange(0.0f));
		s >> Member< float >(L"fogRange", fogRange, AttributeRange(0.0f));
		s >> Member< Color4ub >(L"fogColor", fogColor);
	}

	if (s.getVersion() >= 15)
		s >> MemberEnum< AntiAliasQuality >(L"antiAliasQuality", antiAliasQuality, c_AntiAliasQuality_Keys);

	return true;
}

	}
}
