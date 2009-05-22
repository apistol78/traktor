#ifndef traktor_world_WorldRenderSettings_H
#define traktor_world_WorldRenderSettings_H

#include "Core/Serialization/Serializable.h"
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
class T_DLLCLASS WorldRenderSettings : public Serializable
{
	T_RTTI_CLASS(WorldRenderSettings)

public:
	float viewNearZ;
	float viewFarZ;
	bool depthPassEnabled;
	bool shadowsEnabled;
	int32_t shadowCascadingSlices;
	float shadowCascadingLambda;
	float shadowFarZ;
	int32_t shadowMapResolution;
	float shadowMapBias;

	WorldRenderSettings();

	virtual bool serialize(Serializer& s);
};

	}
}

#endif	// traktor_world_WorldRenderSettings_H
