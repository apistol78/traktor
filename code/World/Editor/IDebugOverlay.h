/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class RenderGraph;
class ScreenRenderer;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class IWorldRenderer;
class World;
class WorldRenderView;

/*! Debug render overlay interface.
 * \ingroup World
 *
 * Debug overlays are used by the editor to visualize various
 * internal buffers for debugging.
 * 
 * Specialized overlays exist for common buffers such as gbuffer channels,
 * shadow maps, velocity buffer etc.
 */
class T_DLLCLASS IDebugOverlay : public Object
{
    T_RTTI_CLASS;

public:
    virtual bool create(resource::IResourceManager* resourceManager) = 0;

    virtual void setup(render::RenderGraph& renderGraph, render::ScreenRenderer* screenRenderer, World* world, IWorldRenderer* worldRenderer, const WorldRenderView& worldRenderView, float alpha, float mip) const = 0;
};

}
