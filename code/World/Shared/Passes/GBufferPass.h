#pragma once

#include "Core/Object.h"
#include "Render/Types.h"
#include "World/WorldRenderSettings.h"

namespace traktor::render
{

class IRenderTargetSet;
class RenderGraph;

}

namespace traktor::world
{

class Entity;
class WorldEntityRenderers;
class WorldRenderView;

/*!
 */
class GBufferPass : public Object
{
    T_RTTI_CLASS;

public:
    explicit GBufferPass(
        const WorldRenderSettings& settings,
        WorldEntityRenderers* entityRenderers,
        render::IRenderTargetSet* sharedDepthStencil)
    ;

	render::handle_t setup(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		const GatherView& gatheredView,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId
	) const;

private:
    WorldRenderSettings m_settings;
    Ref< WorldEntityRenderers > m_entityRenderers;
    Ref< render::IRenderTargetSet > m_sharedDepthStencil;
};

}
