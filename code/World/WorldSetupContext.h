/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "World/WorldTypes.h"

//#remove
#include "Render/IAccelerationStructure.h"

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
		AlignedVector< render::RGDependency >& outVisualAttachments,
		AlignedVector< render::RGDependency >& outSetupAttachments
	);

	const World* getWorld() const { return m_world; }

	const WorldEntityRenderers* getEntityRenderers() const { return m_entityRenderers; }

	render::RenderGraph& getRenderGraph() const { return m_renderGraph; }

	void addVisualAttachment(render::RGDependency visualAttachment) const { m_visualAttachments.push_back(visualAttachment); }

	/*! Add a dependency of setup work produced by an entity renderer.
	 *
	 * All passes drawing the gathered entities take the setup attachments as
	 * inputs; use this to make drawing wait for asynchronous setup work such
	 * as skinning.
	 */
	void addSetupAttachment(render::RGDependency setupAttachment) const { m_setupAttachments.push_back(setupAttachment); }

	/*! Shared dependency of all bottom level acceleration structure updates.
	 *
	 * Entity renderers updating bottom level acceleration structures output
	 * this dependency from their passes; the top level acceleration structure
	 * build consumes it.
	 */
	render::RGDependency getAccelerationStructureDependency() const { return m_asDependency; }

	/*! Dependency of the ray tracing world (top level acceleration structure).
	 *
	 * The top level acceleration structure build outputs this dependency;
	 * passes tracing rays against the world consume it.
	 */
	render::RGDependency getRTWorldDependency() const { return m_rtWorldDependency; }

private:
	const World* m_world;
	const WorldEntityRenderers* m_entityRenderers;
	render::RenderGraph& m_renderGraph;
	AlignedVector< render::RGDependency >& m_visualAttachments;
	AlignedVector< render::RGDependency >& m_setupAttachments;
	render::RGDependency m_asDependency;
	render::RGDependency m_rtWorldDependency;
};

}
