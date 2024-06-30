/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
class ITexture;

}

namespace traktor::world
{

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
		Fixed,
		Adaptive
	};

	struct ShadowSettings
	{
		float farZ = 100.0f;
		int32_t resolution = 1024;
		float bias = 0.01f;
		int32_t cascadingSlices = 4;
		float cascadingLambda = 1.0f;
		bool quantizeProjection = true;
		int32_t maskDenominator = 1;
		resource::Id< render::ImageGraph > maskProject;

		void serialize(ISerializer& s);
	};

	float viewNearZ = 1.0f;
	float viewFarZ = 100.0f;
	ExposureMode exposureMode = Fixed;
	float exposure = 1.0f;
	ShadowSettings shadowSettings[(int)Quality::Last];
	float fogDistance = 90.0f;
	float fogDensity = 0.0f;
	float fogDensityMax = 1.0f;
	Color4f fogColor = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	resource::Id< render::ImageGraph > imageProcess[(int)Quality::Last];
	resource::Id< render::ITexture > colorGrading;

	virtual void serialize(ISerializer& s) override final;
};

}
