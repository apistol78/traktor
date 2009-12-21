#ifndef traktor_world_WorldRenderSettings_H
#define traktor_world_WorldRenderSettings_H

#include "Core/Serialization/ISerializable.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Color.h"
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
	enum ShadowQuality
	{
		SqNoFilter,
		SqLow,
		SqMedium,
		SqHigh
	};

	float viewNearZ;
	float viewFarZ;
	bool depthPassEnabled;
	bool velocityPassEnable;
	bool shadowsEnabled;
	ShadowQuality shadowsQuality;
	bool ssaoEnabled;
	float shadowFarZ;
	int32_t shadowMapResolution;
	float shadowMapBias;

	WorldRenderSettings();

	virtual bool serialize(ISerializer& s);
};

	}
}

#endif	// traktor_world_WorldRenderSettings_H
