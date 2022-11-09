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

namespace traktor::render
{

class ImageGraph;

}

namespace traktor::world
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
	enum ExposureMode
	{
		EmFixed,
		EmAdaptive
	};

	enum ShadowProjection
	{
		SpBox,
		SpLiSP,
		SpTrapezoid,
		SpUniform
	};

	struct ShadowSettings
	{
		ShadowProjection projection = SpUniform;
		float farZ = 0.0f;
		int32_t resolution = 1024;
		float bias = 0.0f;
		float biasCoeff = 1.0f;
		int32_t cascadingSlices = 1;
		float cascadingLambda = 0.0f;
		bool quantizeProjection = false;
		int32_t maskDenominator = 1;
		resource::Id< render::ImageGraph > maskProject;

		void serialize(ISerializer& s);
	};

	float viewNearZ = 1.0f;
	float viewFarZ = 100.0f;
	ExposureMode exposureMode = EmFixed;
	float exposure = 1.0f;
	ShadowSettings shadowSettings[(int)Quality::Last];
	float fogDistance = 90.0f;
	float fogDensity = 0.0f;
	Color4f fogColor = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	resource::Id< IrradianceGrid > irradianceGrid;
	resource::Id< render::ImageGraph > imageProcess[(int)Quality::Last];

	virtual void serialize(ISerializer& s) override final;
};

}
