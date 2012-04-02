#ifndef traktor_world_WorldRenderSettings_H
#define traktor_world_WorldRenderSettings_H

#include "Core/Serialization/ISerializable.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Vector4.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief World render settings.
 * \ingroup World
 *
 * Global render settings used by the WorldRenderer
 * to properly render the scene.
 * This class is designed to be used to store render
 * specific settings.
 */
class T_DLLCLASS WorldRenderSettings : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum RenderType
	{
		RtForward,
		RtPreLit
	};

	enum ShadowProjection
	{
		SpBox,
		SpLiSP,
		SpTrapezoid,
		SpUniform
	};

	enum ShadowQuality
	{
		SqNoFilter,
		SqLow,
		SqMedium,
		SqHigh,
		SqHighest
	};

	RenderType renderType;
	float viewNearZ;
	float viewFarZ;
	bool depthPassEnabled;
	bool shadowsEnabled;
	ShadowProjection shadowsProjection;
	ShadowQuality shadowsQuality;
	float shadowFarZ;
	int32_t shadowMapResolution;
	float shadowMapBias;
	int32_t shadowCascadingSlices;
	float shadowCascadingLambda;
	bool shadowQuantizeProjection;
	bool fogEnabled;
	float fogDistance;
	float fogRange;
	Color4ub fogColor;

	WorldRenderSettings();

	WorldRenderSettings(const WorldRenderSettings& settings);

	virtual bool serialize(ISerializer& s);
};

	}
}

#endif	// traktor_world_WorldRenderSettings_H
