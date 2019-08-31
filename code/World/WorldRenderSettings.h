#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Core/Math/Color4f.h"
#include "Resource/Id.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ImageProcessSettings;

	}

	namespace world
	{

class IrradianceGrid;

/*! World render settings.
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
	enum ShadowProjection
	{
		SpBox,
		SpLiSP,
		SpTrapezoid,
		SpUniform
	};

	struct ShadowSettings
	{
		ShadowProjection projection;
		float farZ;
		int32_t resolution;
		float bias;
		float biasCoeff;
		int32_t cascadingSlices;
		float cascadingLambda;
		bool quantizeProjection;
		int32_t maskDenominator;
		resource::Id< render::ImageProcessSettings > maskProject;

		ShadowSettings();

		void serialize(ISerializer& s);
	};

	float viewNearZ;
	float viewFarZ;
	bool linearLighting;
	float exposureBias;
	ShadowSettings shadowSettings[QuLast];
	bool fog;
	float fogDistanceY;
	float fogDistanceZ;
	float fogDensityY;
	float fogDensityZ;
	Color4f fogColor;
	resource::Id< IrradianceGrid > irradianceGrid;
	resource::Id< render::ImageProcessSettings > imageProcess[QuLast];

	WorldRenderSettings();

	virtual void serialize(ISerializer& s) override final;
};

	}
}
