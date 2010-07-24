#include "World/WorldRenderSettings.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRenderSettings", 5, WorldRenderSettings, ISerializable)

WorldRenderSettings::WorldRenderSettings()
:	viewNearZ(1.0f)
,	viewFarZ(100.0f)
,	depthPassEnabled(true)
,	velocityPassEnable(false)
,	shadowsEnabled(false)
,	shadowsProjection(SpUniform)
,	shadowsQuality(SqMedium)
,	ssaoEnabled(false)
,	shadowFarZ(100.0f)
,	shadowMapResolution(1024)
,	shadowMapBias(0.001f)
{
}

bool WorldRenderSettings::serialize(ISerializer& s)
{
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

	s >> Member< float >(L"viewNearZ", viewNearZ, 0.0f);
	s >> Member< float >(L"viewFarZ", viewFarZ, 0.0f);
	s >> Member< bool >(L"depthPassEnabled", depthPassEnabled);
	
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"velocityPassEnable", velocityPassEnable);

	s >> Member< bool >(L"shadowsEnabled", shadowsEnabled);

	if (s.getVersion() >= 5)
		s >> MemberEnum< ShadowProjection >(L"shadowsProjection", shadowsProjection, c_ShadowProjection_Keys);

	if (s.getVersion() >= 4)
		s >> MemberEnum< ShadowQuality >(L"shadowsQuality", shadowsQuality, c_ShadowQuality_Keys);

	if (s.getVersion() >= 3)
		s >> Member< bool >(L"ssaoEnabled", ssaoEnabled);

	if (s.getVersion() <= 1)
	{
		int32_t shadowCascadingSlices; float shadowCascadingLambda;
		s >> Member< int32_t >(L"shadowCascadingSlices", shadowCascadingSlices);
		s >> Member< float >(L"shadowCascadingLambda", shadowCascadingLambda, 0.0f, 1.0f);
	}

	s >> Member< float >(L"shadowFarZ", shadowFarZ, 0.0f);
	s >> Member< int32_t >(L"shadowMapResolution", shadowMapResolution);
	s >> Member< float >(L"shadowMapBias", shadowMapBias);

	return true;
}

	}
}
