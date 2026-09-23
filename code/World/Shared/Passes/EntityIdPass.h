/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Ref.h"
#include "Render/Frame/RenderGraphTypes.h"
#include "Resource/Proxy.h"

#include <functional>

namespace traktor::render
{

class IRenderSystem;
class RenderGraph;
class Shader;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class EntityIdQuery;
struct EntityState;
class World;
struct WorldCreateDesc;
class WorldEntityRenderers;
class WorldRenderView;

/*! Entity id pass.
 * \ingroup World
 *
 * Renders the id of every entity, through the "World_EntityIdWrite" technique, into a
 * target sharing depth with the world renderer's depth of visible surfaces, such as the
 * g-buffer, then copies the id at the position requested by an entity id query into a
 * read back buffer. Only created for world renderers given a query, which only the editor
 * does, and only rendered the frame after a request.
 */
class EntityIdPass : public Object
{
	T_RTTI_CLASS;

public:
	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc);

	void destroy();

	/*! Check if entity ids are rendered by next setup. */
	bool isRequested() const;

	void setup(
		const World* world,
		const WorldRenderView& worldRenderView,
		const AlignedVector< render::RGDependency >& setupAttachments,
		render::RenderGraph& renderGraph,
		render::RGTargetSet depthTargetSetId,
		render::RGTargetSet visualTargetSetId,
		const std::function< bool(const EntityState& state) >& filter) const;

private:
	Ref< WorldEntityRenderers > m_entityRenderers;
	Ref< EntityIdQuery > m_query;
	resource::Proxy< render::Shader > m_readBackShader;
};

}
