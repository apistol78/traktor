/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IEntityRenderer;
class IrradianceGrid;
class LightComponent;
class ProbeComponent;
class VolumetricFogComponent;

enum
{
	MaxSliceCount = 4,
	MaxLightShadowCount = 2,
	MaxLightsPerCluster = 8,
	ClusterDimXY = 16,
	ClusterDimZ = 32
};

enum class Cancel
{
	Immediate = 0,
	End = 1
};

enum class Quality
{
	Disabled = 0,
	Low = 1,
	Medium = 2,
	High = 3,
	Ultra = 4,
	Last = 5
};

enum class Projection
{
	Orthographic = 0,
	Perspective = 1
};

enum class LightType
{
	Disabled = 0,
	Directional = 1,
	Point = 2,
	Spot = 3
};

/*!
 */
struct T_DLLCLASS EntityState
{
	bool visible = true;
	bool dynamic = false;
	bool locked = false;

	const static EntityState None;
	const static EntityState Visible;
	const static EntityState Dynamic;
	const static EntityState Locked;
	const static EntityState All;
};

/*! Update parameters. */
struct UpdateParams
{
	Object* contextObject = nullptr;	/*!< Update context object; is Stage instance during runtime. */
	double totalTime = 0.0;				/*!< Total time since first update. */
	double alternateTime = 0.0;			/*!< Alternative absolute time. */
	double deltaTime = 0.0;				/*!< Delta time since last update. */
};

/*!
 */
struct GatherView
{
	struct Renderable
	{
		IEntityRenderer* renderer;
		Object* renderable;
		EntityState state;
	};	

    AlignedVector< Renderable > renderables;
	AlignedVector< const LightComponent* > lights;
	AlignedVector< const ProbeComponent* > probes;
	const VolumetricFogComponent* fog;
	const IrradianceGrid* irradianceGrid;
};

/*!
 */
struct DoubleBufferedTarget
{
	render::handle_t previous;
	render::handle_t current;
};

/*! */
Vector2 jitter(int32_t count);

}
