/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"
#include "Render/Types.h"
#include "Terrain/TerrainSurfaceAlloc.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class IRenderSystem;
class IRenderTargetSet;
class ITexture;
class ScreenRenderer;
class RenderContext;
class RenderBlock;
class RenderGraph;

}

namespace traktor::terrain
{

class Terrain;

/*! Terrain surface cache manager.
 * \ingroup Terrain
 */
class T_DLLCLASS TerrainSurfaceCache : public Object
{
	T_RTTI_CLASS;

public:
	virtual ~TerrainSurfaceCache();

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, uint32_t size);

	void destroy();

	void flush();

	bool setupBaseColor(
		render::RenderGraph& renderGraph,
		Terrain* terrain,
		const Vector4& worldOrigin,
		const Vector4& worldExtent
	);

	render::ITexture* getBaseTexture() const;

private:
	resource::IResourceManager* m_resourceManager;
	render::IRenderSystem* m_renderSystem;
	Ref< render::ScreenRenderer > m_screenRenderer;
	Ref< render::IRenderTargetSet > m_base;
	bool m_haveBase = false;
	uint32_t m_updateCount = 0;
};

}
