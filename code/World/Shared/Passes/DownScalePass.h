/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Render/Frame/RenderGraphTypes.h"
#include "Resource/Proxy.h"

namespace traktor::render
{

class Buffer;
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

class Entity;
class WorldRenderView;

/*! Half resolution down scale of the g-buffer, and reduction of its depth range.
 *
 * Both read the g-buffer's view space depth, and both are compute, so they share a
 * single pass. The reduction has no render graph output of its own - it writes only to
 * a read back buffer - and a pass without an output is scheduled as a graph root, which
 * is late in the frame and keeps the transient g-buffer alive until then. Riding along
 * here avoids that; note that compute queued while building a pass runs ahead of that
 * pass' own draws, so the host has to be a pass which executes after the g-buffer pass,
 * not the g-buffer pass itself.
 */
class DownScalePass : public Object
{
	T_RTTI_CLASS;

public:
	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	void destroy();

	/*! Add down scale pass, and reduce the g-buffer depth into a min/max range.
	 *
	 * \param worldRenderView World render view; its index selects the per view state.
	 * \param renderGraph Render graph.
	 * \param gbufferTargetSetId G-buffer; color target 0 red channel holds view space depth.
	 * \param maxReceiverZ Depth beyond which a surface cannot receive a cascade shadow;
	 *                     anything at or past it is excluded from the range. Must account
	 *                     for both the shadow far distance and the depth the g-buffer is
	 *                     cleared to, so neither distant geometry nor sky widens the range.
	 */
	render::RGTexture setup(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::RGTargetSet gbufferTargetSetId);

	/*! Get the depth range measured for a view.
	 *
	 * The range describes what the camera actually sees, as opposed to what its frustum
	 * could contain. It is measured on the GPU, so the value is the one read back during
	 * the most recent setup() and lags the current frame by a few frames; callers are
	 * expected to widen it.
	 *
	 * \param worldRenderView World render view; its index selects the per view state.
	 * \param outMinZ Nearest visible view space depth.
	 * \param outMaxZ Farthest visible view space depth.
	 * \return True if a range has been measured; false until the first result has arrived,
	 *         or when nothing but background was visible.
	 */
	bool getDepthRange(const WorldRenderView& worldRenderView, float& outMinZ, float& outMaxZ) const;

private:
	struct View
	{
		Ref< render::Buffer > rangeBuffer;
		uint32_t frameCount = 0;
		float minZ = 0.0f;
		float maxZ = 0.0f;
		bool valid = false;
	};

	Ref< render::IRenderSystem > m_renderSystem;
	resource::Proxy< render::Shader > m_downScaleShader;
	resource::Proxy< render::Shader > m_depthRangeShader;
	View m_views[4];

	int32_t processDepthRangeReadBack(View& view);
};

}
