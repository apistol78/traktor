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

class RenderGraph;

}

namespace traktor::world
{

class Entity;
class World;
class WorldEntityRenderers;

/*! World setup context.
 * \ingroup World
 */
class T_DLLCLASS WorldSetupContext : public Object
{
	T_RTTI_CLASS;

public:
	explicit WorldSetupContext(
		const World* world,
		const WorldEntityRenderers* entityRenderers,
		render::RenderGraph& renderGraph,
		AlignedVector< render::handle_t >& visualAttachments
	);

	const World* getWorld() const { return m_world; }

	const WorldEntityRenderers* getEntityRenderers() const { return m_entityRenderers; }

	render::RenderGraph& getRenderGraph() const { return m_renderGraph; }

	AlignedVector< render::handle_t >& getVisualAttachments() const { return m_visualAttachments; }

private:
	const World* m_world;
	const WorldEntityRenderers* m_entityRenderers;
	render::RenderGraph& m_renderGraph;
	AlignedVector< render::handle_t >& m_visualAttachments;
};

}
