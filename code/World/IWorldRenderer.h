/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Const.h"
#include "Render/Types.h"
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

class ImageGraphContext;
class IRenderSystem;
class IRenderTargetSet;
class RenderGraph;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class Entity;
class IrradianceGrid;
class World;
class WorldEntityRenderers;
class WorldRenderSettings;
class WorldRenderView;

/*! World renderer quality settings.
 * \ingroup World
 */
struct QualitySettings
{
	Quality toneMap = Quality::Medium;
	Quality motionBlur = Quality::Disabled;
	Quality shadows = Quality::Disabled;
	Quality reflections = Quality::Disabled;
	Quality ambientOcclusion = Quality::Disabled;
	Quality antiAlias = Quality::Disabled;
	Quality imageProcess = Quality::Disabled;
};

/*! World renderer creation description.
 * \ingroup World
 */
struct WorldCreateDesc
{
	const WorldRenderSettings* worldRenderSettings = nullptr;
	WorldEntityRenderers* entityRenderers = nullptr;
	QualitySettings quality;
	uint32_t multiSample = 0;
	float gamma = 2.2f;
	const IrradianceGrid* irradianceGrid = nullptr;			/*!< Explicit irradiance grid, override settings. */
};

/*! World renderer.
 * \ingroup World
 *
 * The world renderer is a high level renderer which render
 * entities through specialized entity renderer.
 * In order to maximize throughput the world renderer is designed with
 * threading and multiple cores in mind as the rendering is split
 * into two parts, one culling and collecting part and one actual rendering
 * part.
 */
class T_DLLCLASS IWorldRenderer : public Object
{
	T_RTTI_CLASS;

public:
	/*! Create world renderer. */
	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) = 0;

	/*! Destroy world renderer. */
	virtual void destroy() = 0;

	/*! \name Build steps. */
	//@{

	/*! Setup render passes.
	 *
	 * \param world World.
	 * \param worldRenderView World render view.
	 * \param renderGraph Setup into render graph.
	 * \param outputTargetSetId ID of output target set.
	 * \param filter Gather only entities which match any filter bits.
	 */
	virtual void setup(
		const World* world,
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		EntityState filter = EntityState::All
	) = 0;

	//@}
};

}
