#include "World/WorldRenderSettings.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.WorldRenderSettings", WorldRenderSettings, Serializable)

WorldRenderSettings::WorldRenderSettings()
:	viewNearZ(1.0f)
,	viewFarZ(100.0f)
,	depthPassEnabled(true)
,	velocityPassEnable(false)
,	shadowsEnabled(false)
,	shadowCascadingSlices(4)
,	shadowCascadingLambda(0.75f)
,	shadowFarZ(50.0f)
,	shadowMapResolution(1024)
,	shadowMapBias(0.001f)
{
}

int WorldRenderSettings::getVersion() const
{
	return 1;
}

bool WorldRenderSettings::serialize(Serializer& s)
{
	s >> Member< float >(L"viewNearZ", viewNearZ, 0.0f);
	s >> Member< float >(L"viewFarZ", viewFarZ, 0.0f);
	s >> Member< bool >(L"depthPassEnabled", depthPassEnabled);
	
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"velocityPassEnable", velocityPassEnable);

	s >> Member< bool >(L"shadowsEnabled", shadowsEnabled);
	s >> Member< int32_t >(L"shadowCascadingSlices", shadowCascadingSlices);
	s >> Member< float >(L"shadowCascadingLambda", shadowCascadingLambda, 0.0f, 1.0f);
	s >> Member< float >(L"shadowFarZ", shadowFarZ, 0.0f);
	s >> Member< int32_t >(L"shadowMapResolution", shadowMapResolution);
	s >> Member< float >(L"shadowMapBias", shadowMapBias);

	return true;
}

	}
}
