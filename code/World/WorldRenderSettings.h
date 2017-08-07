/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_WorldRenderSettings_H
#define traktor_world_WorldRenderSettings_H

#include "Core/Serialization/ISerializable.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Vector4.h"
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
class ITexture;

	}

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
		resource::Id< render::ImageProcessSettings > maskFilter;

		ShadowSettings();

		void serialize(ISerializer& s);
	};

	float viewNearZ;
	float viewFarZ;
	bool linearLighting;
	bool occlusionCulling;
	bool depthPass;
	ShadowSettings shadowSettings[QuLast];
	bool fog;
	float fogDistanceY;
	float fogDistanceZ;
	float fogDensityY;
	float fogDensityZ;
	Color4ub fogColor;
	resource::Id< render::ITexture > reflectionMap;
	resource::Id< render::ImageProcessSettings > imageProcess[QuLast];

	WorldRenderSettings();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_world_WorldRenderSettings_H
