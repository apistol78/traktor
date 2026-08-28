/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/DownScalePass.h"

#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Misc/String.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityRenderer.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

#include <cstring>

namespace traktor::world
{
namespace
{

const resource::Id< render::Shader > c_downScaleShader(L"{BC74A385-9406-884D-9FB5-EA84F413185C}");
const resource::Id< render::Shader > c_depthRangeShader(L"{9E0C0BBA-662E-4231-A4D1-FB4AF0064757}"); // World/Depth range/DepthRange

const render::Handle s_handleDownScaleInput(L"World_DownScaleInput");
const render::Handle s_handleDownScaleOutput(L"World_DownScaleOutput");
const render::Handle s_handleDownScaleWorkSize(L"World_DownScaleWorkSize");

const render::Handle s_handleDepthRangeInput(L"World_DepthRangeInput");
const render::Handle s_handleDepthRangeBuffer(L"World_DepthRangeBuffer");
const render::Handle s_handleDepthRangeParams(L"World_DepthRangeParams");

constexpr uint32_t c_depthRangeRingSize = 8;
constexpr uint32_t c_depthRangeLatency = 4;
constexpr int32_t c_depthRangeBlockSize = 8;
constexpr int32_t c_depthRangeResetMinZ = 0x7fffffff;
constexpr int32_t c_depthRangeResetMaxZ = 0;

#pragma pack(1)

struct DepthRange
{
	int32_t minZ;
	int32_t maxZ;
};

#pragma pack()

float asFloat(int32_t value)
{
	float f;
	std::memcpy(&f, &value, sizeof(f));
	return f;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DownScalePass", DownScalePass, Object)

bool DownScalePass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
{
	if (!resourceManager->bind(c_downScaleShader, m_downScaleShader))
	{
		log::error << L"Unable to create down-scale process." << Endl;
		return false;
	}
	if (!resourceManager->bind(c_depthRangeShader, m_depthRangeShader))
	{
		log::error << L"Unable to create depth range process." << Endl;
		return false;
	}
	m_renderSystem = renderSystem;
	return true;
}

void DownScalePass::destroy()
{
	for (auto& view : m_views)
		if (view.rangeBuffer)
		{
			view.rangeBuffer->destroy();
			view.rangeBuffer = nullptr;
		}
	m_depthRangeShader.clear();
	m_downScaleShader.clear();
	m_renderSystem = nullptr;
}

render::RGTexture DownScalePass::setup(
	const WorldRenderView& worldRenderView,
	render::RenderGraph& renderGraph,
	render::RGTargetSet gbufferTargetSetId)
{
	T_PROFILER_SCOPE(L"DownScalePass::setup");

	const int32_t viewIndex = worldRenderView.getIndex();
	T_FATAL_ASSERT(viewIndex >= 0 && viewIndex < sizeof_array(m_views));

	View& view = m_views[viewIndex];
	const int32_t writeSlot = processDepthRangeReadBack(view);
	Ref< render::Buffer > rangeBuffer = view.rangeBuffer;

	const render::RenderGraphTextureDesc rgtxd = {
		.referenceWidthDenom = 2,
		.referenceHeightDenom = 2,
		.mipCount = 1,
		.format = render::TfR16G16B16A16F
	};
	const render::RGTexture downScaleTextureId = renderGraph.addTransientTexture(L"DownScale", rgtxd, gbufferTargetSetId);

	Ref< render::RenderPass > rp = new render::RenderPass(L"DownScale");
	rp->addInput(gbufferTargetSetId);
	rp->setOutput(downScaleTextureId);
	rp->addBuild(
		[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		render::ITexture* inputTexture = renderGraph.getTargetSet(gbufferTargetSetId)->getColorTexture(0);
		render::ITexture* outputTexture = renderGraph.getTexture(downScaleTextureId);

		const render::ITexture::Size inputSize = inputTexture->getSize();
		const render::ITexture::Size outputSize = outputTexture->getSize();

		{
			auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"DownScale");

			renderBlock->program = m_downScaleShader->getProgram().program;
			renderBlock->workSize[0] = outputSize.x;
			renderBlock->workSize[1] = outputSize.y;
			renderBlock->workSize[2] = 1;

			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setVectorParameter(s_handleDownScaleWorkSize, Vector4(outputSize.x, outputSize.y, inputSize.x, inputSize.y));
			renderBlock->programParams->setImageViewParameter(s_handleDownScaleOutput, outputTexture, 0);
			renderBlock->programParams->setImageViewParameter(s_handleDownScaleInput, inputTexture, 0);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
			renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute, outputTexture, 0);
		}

		// Reduce the g-buffer depth to a min/max range. Reads the g-buffer, not the down
		// scaled output, so it needs no barrier against the above.
		if (writeSlot >= 0)
		{
			const int32_t inputWidth = (int32_t)inputSize.x;
			const int32_t inputHeight = (int32_t)inputSize.y;

			auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Depth range");

			renderBlock->program = m_depthRangeShader->getProgram().program;
			renderBlock->workSize[0] = (inputWidth + c_depthRangeBlockSize - 1) / c_depthRangeBlockSize;
			renderBlock->workSize[1] = (inputHeight + c_depthRangeBlockSize - 1) / c_depthRangeBlockSize;
			renderBlock->workSize[2] = 1;

			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setImageViewParameter(s_handleDepthRangeInput, inputTexture, 0);
			renderBlock->programParams->setBufferViewParameter(s_handleDepthRangeBuffer, rangeBuffer->getBufferView());
			renderBlock->programParams->setVectorParameter(s_handleDepthRangeParams, Vector4((float)inputWidth, (float)inputHeight, std::numeric_limits< float >::max(), (float)writeSlot));
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
		}
	});
	renderGraph.addPass(rp);

	return downScaleTextureId;
}

bool DownScalePass::getDepthRange(const WorldRenderView& worldRenderView, float& outMinZ, float& outMaxZ) const
{
	const int32_t viewIndex = worldRenderView.getIndex();
	T_FATAL_ASSERT(viewIndex >= 0 && viewIndex < sizeof_array(m_views));

	const View& view = m_views[viewIndex];
	if (!view.valid)
		return false;

	outMinZ = view.minZ;
	outMaxZ = view.maxZ;
	return true;
}

int32_t DownScalePass::processDepthRangeReadBack(View& view)
{
	if (!view.rangeBuffer)
	{
		view.rangeBuffer = m_renderSystem->createBuffer(
			render::BufferUsage::BuStructured | render::BufferUsage::BuReadBack,
			c_depthRangeRingSize * sizeof(DepthRange),
			false,
			T_FILE_LINE_W);
		if (!view.rangeBuffer)
			return -1;

		// Freshly allocated memory is undefined; reset the whole ring so it is never
		// mistaken for a measurement.
		auto* initial = (DepthRange*)view.rangeBuffer->lock();
		if (!initial)
		{
			log::error << L"Depth range buffer is not readable; depth range unavailable." << Endl;
			view.rangeBuffer = nullptr;
			return -1;
		}
		for (uint32_t i = 0; i < c_depthRangeRingSize; ++i)
		{
			initial[i].minZ = c_depthRangeResetMinZ;
			initial[i].maxZ = c_depthRangeResetMaxZ;
		}
		view.rangeBuffer->unlock();
	}

	// Read an earlier frame's result and reset the slot this frame is about to reduce
	// into. Locking a read back buffer maps its own storage, so the GPU's writes are
	// visible and resetting one slot leaves the rest of the ring alone.
	auto* slots = (DepthRange*)view.rangeBuffer->lock();
	if (!slots)
		return -1;

	// Take the freshest slot which the GPU has finished writing. A slot which is still in
	// flight is left at its reset values and is skipped, so a frame or two of extra GPU
	// latency costs staleness rather than dropping the range entirely.
	view.valid = false;
	for (uint32_t age = c_depthRangeLatency; age < c_depthRangeRingSize && !view.valid; ++age)
	{
		if (age > view.frameCount)
			break;

		const DepthRange& range = slots[(view.frameCount + c_depthRangeRingSize - age) % c_depthRangeRingSize];

		// Nothing but background in view also leaves the slot at its reset values; either
		// way there is no range to be had from it.
		if (range.maxZ <= 0 || range.minZ > range.maxZ)
			continue;

		const float minZ = asFloat(range.minZ);
		const float maxZ = asFloat(range.maxZ);
		if (minZ > 0.0f && maxZ >= minZ)
		{
			view.minZ = minZ;
			view.maxZ = maxZ;
			view.valid = true;
		}
	}

	const uint32_t writeSlot = view.frameCount % c_depthRangeRingSize;
	slots[writeSlot].minZ = c_depthRangeResetMinZ;
	slots[writeSlot].maxZ = c_depthRangeResetMaxZ;

	view.rangeBuffer->unlock();

	view.frameCount++;
	return (int32_t)writeSlot;
}

}
