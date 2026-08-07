/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Frame/RenderGraph.h"

#include "Core/Containers/StaticSet.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Timer/Profiler.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraphBufferPool.h"
#include "Render/Frame/RenderGraphContext.h"
#include "Render/Frame/RenderGraphTargetSet.h"
#include "Render/Frame/RenderGraphTargetSetPool.h"
#include "Render/Frame/RenderGraphTexturePool.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"

namespace traktor::render
{
namespace
{

void traverse(const RefArray< const RenderPass >& passes, int32_t depth, int32_t index, StaticVector< uint32_t, 512 >& chain, const std::function< void(int32_t, int32_t) >& fn)
{
	// Check if we're in a cyclic path.
	if (std::find(chain.begin(), chain.end(), index) != chain.end())
		return;

	// Traverse inputs first as we want to traverse passes depth-first.
	chain.push_back(index);
	for (const auto& input : passes[index]->getInputs())
	{
		for (int32_t i = 0; i < passes.size(); ++i)
			if (passes[i]->getOutput().resourceId == input.resourceId)
				traverse(passes, depth + 1, i, chain, fn);
	}
	chain.pop_back();

	// Call visitor for this pass.
	fn(depth, index);
}

bool isExplicitTargetSetId(RGTargetSet targetSetId)
{
	return targetSetId != RGTargetSet::Output && targetSetId != RGTargetSet::Invalid;
}

const RenderGraph::TargetResource* findTargetResource(const SmallMap< RGTargetSet, RenderGraph::TargetResource >& targets, RGTargetSet targetSetId)
{
	const auto it = targets.find(targetSetId);
	return it != targets.end() ? &it->second : nullptr;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderGraph", RenderGraph, Object)

RenderGraph::RenderGraph(
	RenderGraphContext* context,
	uint32_t multiSample,
	const fn_profiler_t& profiler)
	: m_context(context)
	, m_counter(0)
	, m_multiSample(multiSample)
	, m_nextResourceId(1)
	, m_profiler(profiler)
	, m_ownContext(false)
{
}

RenderGraph::RenderGraph(
	IRenderSystem* renderSystem,
	uint32_t multiSample,
	const fn_profiler_t& profiler)
	: m_context(new RenderGraphContext(renderSystem))
	, m_counter(0)
	, m_multiSample(multiSample)
	, m_nextResourceId(1)
	, m_profiler(profiler)
	, m_ownContext(true)
{
}

void RenderGraph::destroy()
{
	m_targets.clear();
	m_buffers.clear();
	m_passes.clear();
	for (int32_t i = 0; i < sizeof_array(m_order); ++i)
		m_order[i].clear();

	if (m_ownContext)
		safeDestroy(m_context);

	m_context = nullptr;
}

RGTargetSet RenderGraph::addExplicitTargetSet(const wchar_t* const name, IRenderTargetSet* targetSet)
{
	const RGTargetSet resourceId(m_nextResourceId++);

	auto& tr = m_targets[resourceId];
	tr.name = name;
	tr.persistentHandle = 0;
	tr.doubleBuffered = false;
	tr.targetSet = new RenderGraphTargetSet(targetSet, targetSet);
	tr.sizeReferenceTargetSetId = RGTargetSet::Invalid;
	tr.inputRefCount = 0;
	tr.outputRefCount = 0;
	tr.external = true;

	return resourceId;
}

RGTargetSet RenderGraph::addTransientTargetSet(
	const wchar_t* const name,
	const RenderGraphTargetSetDesc& targetSetDesc,
	RGTargetSet sharedDepthStencilTargetSetId,
	RGTargetSet sizeReferenceTargetSetId)
{
	const RGTargetSet resourceId(m_nextResourceId++);

	auto& tr = m_targets[resourceId];
	tr.name = name;
	tr.persistentHandle = 0;
	tr.doubleBuffered = false;
	tr.targetSetDesc = targetSetDesc;
	tr.sharedDepthStencilTargetSetId = sharedDepthStencilTargetSetId;
	tr.sizeReferenceTargetSetId = sizeReferenceTargetSetId;
	tr.inputRefCount = 0;
	tr.outputRefCount = 0;
	tr.external = false;

	return resourceId;
}

RGTargetSet RenderGraph::addPersistentTargetSet(
	const wchar_t* const name,
	handle_t persistentHandle,
	bool doubleBuffered,
	const RenderGraphTargetSetDesc& targetSetDesc,
	RGTargetSet sharedDepthStencilTargetSetId,
	RGTargetSet sizeReferenceTargetSetId)
{
	const RGTargetSet resourceId(m_nextResourceId++);

	auto& tr = m_targets[resourceId];
	tr.name = name;
	tr.persistentHandle = persistentHandle;
	tr.doubleBuffered = doubleBuffered;
	tr.targetSetDesc = targetSetDesc;
	tr.sharedDepthStencilTargetSetId = sharedDepthStencilTargetSetId;
	tr.sizeReferenceTargetSetId = sizeReferenceTargetSetId;
	tr.inputRefCount = 0;
	tr.outputRefCount = 0;
	tr.external = false;

	return resourceId;
}

RGBuffer RenderGraph::addExplicitBuffer(const wchar_t* const name, Buffer* buffer)
{
	const RGBuffer resourceId(m_nextResourceId++);

	auto& br = m_buffers[resourceId];
	br.name = name;
	br.buffer = buffer;

	return resourceId;
}

RGBuffer RenderGraph::addTransientBuffer(const wchar_t* const name, const RenderGraphBufferDesc& bufferDesc)
{
	const RGBuffer resourceId(m_nextResourceId++);

	auto& br = m_buffers[resourceId];
	br.name = name;
	br.bufferDesc = bufferDesc;

	return resourceId;
}

RGBuffer RenderGraph::addPersistentBuffer(const wchar_t* const name, handle_t persistentHandle, const RenderGraphBufferDesc& bufferDesc)
{
	const RGBuffer resourceId(m_nextResourceId++);

	auto& br = m_buffers[resourceId];
	br.name = name;
	br.persistentHandle = persistentHandle;
	br.bufferDesc = bufferDesc;

	return resourceId;
}

RGTexture RenderGraph::addExplicitTexture(const wchar_t* const name, ITexture* texture)
{
	const RGTexture resourceId(m_nextResourceId++);

	auto& br = m_textures[resourceId];
	br.name = name;
	br.texture = texture;

	return resourceId;
}

RGTexture RenderGraph::addTransientTexture(const wchar_t* const name, const RenderGraphTextureDesc& textureDesc, RGTargetSet sizeReferenceTargetSetId)
{
	const RGTexture resourceId(m_nextResourceId++);

	auto& tr = m_textures[resourceId];
	tr.name = name;
	tr.textureDesc = textureDesc;
	tr.sizeReferenceTargetSetId = sizeReferenceTargetSetId;

	return resourceId;
}

RGTexture RenderGraph::addPersistentTexture(const wchar_t* const name, handle_t persistentHandle, const RenderGraphTextureDesc& textureDesc, RGTargetSet sizeReferenceTargetSetId)
{
	const RGTexture resourceId(m_nextResourceId++);

	auto& tr = m_textures[resourceId];
	tr.name = name;
	tr.persistentHandle = persistentHandle;
	tr.textureDesc = textureDesc;
	tr.sizeReferenceTargetSetId = sizeReferenceTargetSetId;

	return resourceId;
}

RGDependency RenderGraph::addDependency()
{
	const RGDependency dependency(m_nextResourceId++);
	m_dependencies.insert(dependency.get());
	return dependency;
}

IRenderTargetSet* RenderGraph::getTargetSet(RGTargetSet resource) const
{
	T_FATAL_ASSERT(m_buildingPasses);

	const auto it = m_targets.find(resource);
	if (it == m_targets.end() || it->second.targetSet == nullptr)
		return nullptr;

	return it->second.targetSet->getReadTargetSet();
}

Buffer* RenderGraph::getBuffer(RGBuffer resource) const
{
	T_FATAL_ASSERT(m_buildingPasses);
	const auto it = m_buffers.find(resource);
	return (it != m_buffers.end()) ? it->second.buffer : nullptr;
}

ITexture* RenderGraph::getTexture(RGTexture resource) const
{
	T_FATAL_ASSERT(m_buildingPasses);
	const auto it = m_textures.find(resource);
	return (it != m_textures.end()) ? it->second.texture : nullptr;
}

void RenderGraph::addPass(const RenderPass* pass)
{
	m_passes.push_back(pass);
}

bool RenderGraph::validate()
{
	// Find root passes, which are either of:
	// 1) Have no output resource.
	// 2) Writing to external targets.
	// 3) Writing to primary target.
	StaticVector< uint32_t, 32 > roots;
	for (uint32_t i = 0; i < (uint32_t)m_passes.size(); ++i)
	{
		const auto pass = m_passes[i];
		const auto& output = pass->getOutput();
		if (output.resourceId == ~0)
			roots.push_back(i);
		else if (output.resourceId != ~0 && output.resourceId != 0)
		{
			auto it = m_targets.find(RGTargetSet(output.resourceId));
			if (it != m_targets.end() && it->second.external)
				roots.push_back(i);
		}
		else if (output.resourceId == 0)
			roots.push_back(i);
	}

	// Determine maximum depths of each pass.
	StaticVector< int32_t, 512 > depths;
	depths.resize(m_passes.size(), -1);
	for (auto root : roots)
	{
		StaticVector< uint32_t, 512 > chain;
		traverse(m_passes, 0, root, chain, [&](int32_t depth, int32_t index) {
			T_ASSERT(depth < sizeof_array(m_order));
			depths[index] = std::max(depths[index], depth);
		});
	}

	// Gather passes in order for each depth.
	//
	// Passes are placed in "bands"; build() renders bands high->low, so a higher
	// band is rendered earlier. Natural (auto-ordered) passes are shifted up by one
	// to reserve band 0 for "Last" anchored passes, and the top band for "First"
	// anchored passes, guaranteeing those render strictly after/before everything else.
	const int32_t firstBand = sizeof_array(m_order) - 1;
	const int32_t lastBand = 0;

	const auto hasAnchor = [](const RenderPass* pass, RGDependency anchor) {
		for (const auto& input : pass->getInputs())
			if (input.resourceId == anchor.get())
				return true;
		return false;
	};

	for (int32_t i = 0; i < sizeof_array(m_order); ++i)
		m_order[i].resize(0);
	for (uint32_t i = 0; i < (uint32_t)m_passes.size(); ++i)
	{
		if (depths[i] < 0)
			continue;

		int32_t band;
		if (hasAnchor(m_passes[i], RGDependency::First))
			band = firstBand;
		else if (hasAnchor(m_passes[i], RGDependency::Last))
			band = lastBand;
		else
			band = std::min(depths[i] + 1, firstBand - 1);

		m_order[band].push_back(i);
	}

	// Determine which graphics passes consume outputs of asynchronous compute
	// passes; they are ordered as late as possible within their band so as much
	// other work as possible is recorded before the graphics queue has to wait
	// on the asynchronous results. Asynchronous passes themselves are excluded;
	// they should record, and thus submit, as early as possible.
	StaticSet< handle_t, 64 > asyncOutputs;
	for (uint32_t i = 0; i < (uint32_t)m_passes.size(); ++i)
		if (m_passes[i]->getQueue() == RenderPass::Queue::AsyncCompute && m_passes[i]->getOutput().resourceId != ~0U)
			asyncOutputs.insert(m_passes[i]->getOutput().resourceId);

	StaticVector< bool, 512 > lateKeys;
	lateKeys.resize(m_passes.size(), false);
	if (!asyncOutputs.empty())
	{
		for (uint32_t i = 0; i < (uint32_t)m_passes.size(); ++i)
		{
			if (m_passes[i]->getQueue() == RenderPass::Queue::AsyncCompute)
				continue;
			for (const auto& input : m_passes[i]->getInputs())
				if (asyncOutputs.find(input.resourceId) != asyncOutputs.end())
				{
					lateKeys[i] = true;
					break;
				}
		}

		// Aggregate the flag per output resource within each band so passes merged
		// into the same render pass are kept adjacent and merging stays intact.
		for (int32_t i = 0; i < sizeof_array(m_order); ++i)
		{
			const auto& order = m_order[i];
			for (uint32_t a = 0; a < order.size(); ++a)
			{
				const handle_t oa = m_passes[order[a]]->getOutput().resourceId;
				if (oa == ~0U || !lateKeys[order[a]])
					continue;
				for (uint32_t b = 0; b < order.size(); ++b)
					if (m_passes[order[b]]->getOutput().resourceId == oa)
						lateKeys[order[b]] = true;
			}
		}
	}

	// Sort each depth; passes consuming asynchronous results last, then based
	// on output resource.
	for (int32_t i = 0; i < sizeof_array(m_order); ++i)
		std::stable_sort(m_order[i].begin(), m_order[i].end(), [&](uint32_t lh, uint32_t rh) {
			if (lateKeys[lh] != lateKeys[rh])
				return !lateKeys[lh];
			const auto lt = m_passes[lh]->getOutput().resourceId;
			const auto rt = m_passes[rh]->getOutput().resourceId;
			return lt > rt;
		});

	// Stabilize order within each band so a pass consuming a dependency is
	// recorded after the pass producing it; the sort above only groups passes
	// by output resource and anchored bands would otherwise be ordered by
	// insertion order which is arbitrary. Only dependency resources are used
	// as edges to not disturb the target grouping.
	const auto producesDependencyFor = [&](uint32_t producer, uint32_t consumer) {
		const handle_t output = m_passes[producer]->getOutput().resourceId;
		if (output == ~0U || m_dependencies.find(output) == m_dependencies.end())
			return false;
		for (const auto& input : m_passes[consumer]->getInputs())
			if (input.resourceId == output)
				return true;
		return false;
	};
	for (int32_t i = 0; i < sizeof_array(m_order); ++i)
	{
		auto& order = m_order[i];
		if (order.size() <= 1)
			continue;

		StaticVector< uint32_t, 64 > sorted;
		StaticVector< uint32_t, 64 > pending = order;
		while (!pending.empty())
		{
			// Pick first pass with no pending producer; fall back to first pass in case of a cycle.
			size_t pick = 0;
			for (size_t j = 0; j < pending.size(); ++j)
			{
				bool ready = true;
				for (size_t k = 0; k < pending.size() && ready; ++k)
					if (k != j && producesDependencyFor(pending[k], pending[j]))
						ready = false;
				if (ready)
				{
					pick = j;
					break;
				}
			}
			sorted.push_back(pending[pick]);
			pending.erase(pending.begin() + pick);
		}
		order = sorted;
	}

	// Determine synchronization points for asynchronous compute passes; the
	// graphics queue must wait upon the asynchronous work before the first
	// graphics pass consuming any of its outputs is executed. Since a wait
	// covers all asynchronous work recorded before its signal a single wait
	// before the earliest consumer covers every later consumer as well. A
	// consumer on the asynchronous queue only requires a barrier since that
	// queue executes in recorded order.
	m_syncBefore.reset();
	m_barrierBefore.reset();
	m_waitAfter.reset();
	{
		StaticVector< uint32_t, 512 > linear;
		for (int32_t i = sizeof_array(m_order) - 1; i >= 0; --i)
			for (const auto index : m_order[i])
				linear.push_back(index);

		for (uint32_t i = 0; i < (uint32_t)linear.size(); ++i)
		{
			const RenderPass* producer = m_passes[linear[i]];
			if (producer->getQueue() != RenderPass::Queue::AsyncCompute)
				continue;

			const handle_t output = producer->getOutput().resourceId;

			// Asynchronous compute passes cannot render to targets.
			T_ASSERT(m_targets.find(RGTargetSet(output)) == m_targets.end());

#if defined(_DEBUG)
			// Asynchronous compute passes cannot consume results produced by graphics
			// passes within the same frame; the compute queue cannot wait upon the
			// graphics queue.
			for (const auto& input : producer->getInputs())
				for (uint32_t j = 0; j < (uint32_t)linear.size(); ++j)
					T_ASSERT_M(
						m_passes[linear[j]]->getQueue() != RenderPass::Queue::Graphics || m_passes[linear[j]]->getOutput().resourceId != input.resourceId,
						L"Asynchronous compute pass consumes graphics output; not supported.");
#endif

			bool haveConsumer = false;
			if (output != ~0U)
			{
				for (uint32_t j = i + 1; j < (uint32_t)linear.size(); ++j)
				{
					const RenderPass* consumer = m_passes[linear[j]];

					bool consumes = false;
					for (const auto& input : consumer->getInputs())
						if (input.resourceId == output)
						{
							consumes = true;
							break;
						}
					if (!consumes)
						continue;

					if (consumer->getQueue() == RenderPass::Queue::AsyncCompute)
					{
						m_barrierBefore.insert(linear[j]);
						haveConsumer = true;
					}
					else
					{
						m_syncBefore[linear[j]].push_back(linear[i]);
						haveConsumer = true;
						break;
					}
				}
			}

			// No consumer pass known; conservatively synchronize immediately after the
			// pass to guarantee this frame's graphics work observes the result.
			if (!haveConsumer)
				m_waitAfter.insert(linear[i]);
		}
	}

	// Gather targets which are used as shared depth.
	for (auto& it : m_targets)
		if (
			it.second.sharedDepthStencilTargetSetId != RGTargetSet::Invalid &&
			it.second.sharedDepthStencilTargetSetId != RGTargetSet::Output)
			m_sharedDepthTargets.insert(it.second.sharedDepthStencilTargetSetId);

	return true;
}

bool RenderGraph::build(RenderContext* renderContext, int32_t width, int32_t height)
{
	T_FATAL_ASSERT(!renderContext->havePendingDraws());

	// Initialize input and output reference counts of all targets.
	for (int32_t i = 0; i < sizeof_array(m_order); ++i)
	{
		const auto& order = m_order[i];
		for (const auto index : order)
		{
			const auto pass = m_passes[index];
			const auto& output = pass->getOutput();
			if (output.resourceId != ~0)
			{
				auto it = m_targets.find(RGTargetSet(output.resourceId));
				if (it != m_targets.end())
					it->second.outputRefCount++;
			}
			for (const auto& input : pass->getInputs())
			{
				auto it = m_targets.find(RGTargetSet(input.resourceId));
				if (it != m_targets.end())
					it->second.inputRefCount++;
			}
		}
	}

	// Calculate size of all targets.
	for (auto it : m_targets)
		if (!realizeTargetDimensions(width, height, it.first))
			return false;

	// Acquire all targets which are used for sharing depth.
	for (auto id : m_sharedDepthTargets)
	{
		auto& target = m_targets[id];
		// Promote transient shared-depth targets to frame lifetime, but never
		// downgrade a target the caller made persistent; doing so would route it
		// to the shared ~0U pool and discard its preserved (double-buffered) content.
		if (target.persistentHandle == 0)
			target.persistentHandle = ~0U;
		if (!acquire(target))
		{
			cleanup();
			return false;
		}
	}

	// Acquire all persistent targets in case they are read from
	// before being used as an output.
	for (auto& it : m_targets)
	{
		auto& target = it.second;
		if (
			target.targetSet == nullptr &&
			target.persistentHandle != 0 &&
			(target.inputRefCount != 0 || target.outputRefCount != 0))
		{
			if (!acquire(target))
			{
				cleanup();
				return false;
			}
		}
	}

	for (auto& it : m_buffers)
	{
		auto& sbuffer = it.second;
		if (sbuffer.buffer == nullptr)
		{
			sbuffer.buffer = m_context->getBufferPool()->acquire(sbuffer.bufferDesc, width, height, sbuffer.persistentHandle);
			if (!sbuffer.buffer)
				return false;
		}
	}

	for (auto& it : m_textures)
	{
		auto& texture = it.second;
		if (texture.texture == nullptr)
		{
			int32_t referenceWidth = width;
			int32_t referenceHeight = height;

			if (
				texture.sizeReferenceTargetSetId != RGTargetSet::Invalid &&
				texture.sizeReferenceTargetSetId != RGTargetSet::Output
			)
			{
				auto it = m_targets.find(texture.sizeReferenceTargetSetId);
				if (it != m_targets.end())
				{
					const TargetResource& target = it->second;
					referenceWidth = target.realized.width;
					referenceHeight = target.realized.height;
				}
				else
					log::error << L"No target " << (int32_t)texture.sizeReferenceTargetSetId.get() << Endl;
			}

			texture.texture = m_context->getTexturePool()->acquire(texture.textureDesc, referenceWidth, referenceHeight, texture.persistentHandle);
			if (!texture.texture)
				return false;
		}
	}

#if !defined(__ANDROID__) && !defined(__IOS__)
	double referenceOffset = Profiler::getInstance().getTime();

	int32_t* queryHandles = nullptr;
	int32_t* referenceQueryHandle = nullptr;
	int32_t* passQueryHandles = nullptr;
	int32_t* profiling = nullptr;

	if (m_profiler)
	{
		// Allocate query handles from render context's heap since they get automatically
		// freed when the context is reset.
		queryHandles = (int32_t*)renderContext->alloc((uint32_t)(m_passes.size() + 1) * sizeof(int32_t), (uint32_t)alignOf< int32_t >());
		referenceQueryHandle = queryHandles;
		passQueryHandles = queryHandles + 1;

		renderContext->direct< ProfileBeginRenderBlock >(referenceQueryHandle);
	}

#	define T_PASS_PROFILE_BEGIN() \
		if (profiling) \
		{ \
			renderContext->direct< ProfileBeginRenderBlock >(profiling); \
		}
#	define T_PASS_PROFILE_END() \
		if (profiling) \
		{ \
			renderContext->direct< ProfileEndRenderBlock >(profiling); \
			profiling = nullptr; \
		}
#	define T_PASS_IS_PROFILING \
		(profiling != nullptr)

#else
#	define T_PASS_PROFILE_BEGIN()
#	define T_PASS_PROFILE_END()
#	define T_PASS_IS_PROFILING false
#endif

	// Render passes in dependency order.
	//
	// Since we don't want to load/store render passes, esp when using MSAA,
	// we track current output target and automatically merge render passes.
	//
	TargetResource* currentTarget = nullptr;
	RenderPass::Output currentOutput;

	// Handles of signalled asynchronous compute passes; used to synchronize the
	// graphics queue before the first consumer of each asynchronous pass.
	StaticVector< ComputeHandle*, 512 > asyncComputeHandles;
	asyncComputeHandles.resize(m_passes.size(), nullptr);

	for (int32_t i = sizeof_array(m_order) - 1; i >= 0; --i)
	{
		const auto& order = m_order[i];
		for (const auto index : order)
		{
			const auto pass = m_passes[index];
			const auto& inputs = pass->getInputs();
			const auto& output = pass->getOutput();
			const bool asyncCompute = (pass->getQueue() == RenderPass::Queue::AsyncCompute);
			const auto syncBefore = m_syncBefore.find(index);
			const bool syncPoint = (syncBefore != m_syncBefore.end());
			const bool waitAfter = asyncCompute && (m_waitAfter.find(index) != m_waitAfter.end());

			// Begin render pass. An asynchronous compute pass doesn't interact with the
			// graphics queue, except when it's conservatively waited upon immediately, so
			// any current render pass is left open.
			if (asyncCompute)
			{
				if (waitAfter && currentOutput.resourceId != ~0U)
				{
					T_PASS_PROFILE_BEGIN();
					renderContext->mergeComputeIntoRender();
					renderContext->mergeDrawIntoRender();
					renderContext->direct< EndPassRenderBlock >();
					T_PASS_PROFILE_END();

					if (currentTarget && currentTarget->doubleBuffered)
						currentTarget->targetSet->swap();

					currentTarget = nullptr;
					currentOutput = RenderPass::Output();
				}
			}
			else if (pass->haveOutput())
			{
				if (output.resourceId != 0)
				{
					// Continue rendering to same target if possible; else start another pass.
					if (T_PASS_IS_PROFILING || syncPoint || currentOutput != output)
					{
						if (currentOutput.resourceId != ~0U)
						{
							T_PASS_PROFILE_BEGIN();
							renderContext->mergeComputeIntoRender();
							renderContext->mergeDrawIntoRender();
							renderContext->direct< EndPassRenderBlock >();
							T_PASS_PROFILE_END();

							if (currentTarget && currentTarget->doubleBuffered)
								currentTarget->targetSet->swap();
						}
						else if (T_PASS_IS_PROFILING)
						{
							// Profiling of a non-output pass.
							T_PASS_PROFILE_BEGIN();
							renderContext->mergeComputeIntoRender();
							renderContext->mergeDrawIntoRender();
							T_PASS_PROFILE_END();
						}

						// Begin pass if resource is a target.
						auto it = m_targets.find(RGTargetSet(output.resourceId));
						if (it != m_targets.end())
						{
							TargetResource& target = it->second;
							if (target.targetSet == nullptr)
							{
								T_ASSERT(!target.external);
								if (!acquire(target))
								{
									cleanup();
									return false;
								}
							}

							if (output.pass)
							{
								auto tb = renderContext->allocNamed< BeginPassRenderBlock >(pass->getName());
								tb->renderTargetSet = target.targetSet->getWriteTargetSet();
								tb->clear = output.clear;
								tb->load = output.load;
								tb->store = output.store;
								renderContext->draw(tb);

								currentTarget = &target;
								currentOutput = output;
							}
							else
							{
								currentTarget = nullptr;
								currentOutput = RenderPass::Output();
							}
						}
						else
						{
							currentTarget = nullptr;
							currentOutput = RenderPass::Output();
						}
					}
				}
				else // Output to framebuffer; implicit as target 0.
				{
					if (T_PASS_IS_PROFILING || syncPoint || currentOutput.resourceId != 0)
					{
						if (currentOutput.resourceId != ~0U)
						{
							T_PASS_PROFILE_BEGIN();
							renderContext->mergeComputeIntoRender();
							renderContext->mergeDrawIntoRender();
							renderContext->direct< EndPassRenderBlock >();
							T_PASS_PROFILE_END();

							if (currentTarget && currentTarget->doubleBuffered)
								currentTarget->targetSet->swap();
						}
						else if (T_PASS_IS_PROFILING)
						{
							// Profiling of a non-output pass.
							T_PASS_PROFILE_BEGIN();
							renderContext->mergeComputeIntoRender();
							renderContext->mergeDrawIntoRender();
							T_PASS_PROFILE_END();
						}
						
						auto tb = renderContext->allocNamed< BeginPassRenderBlock >(pass->getName());
						tb->clear = output.clear;
						tb->load = output.load;
						tb->store = output.store;
						renderContext->draw(tb);

						currentTarget = nullptr;
						currentOutput = output;
					}
				}
			}
			else if (currentOutput.resourceId != ~0U)
			{
				T_PASS_PROFILE_BEGIN();
				renderContext->mergeComputeIntoRender();
				renderContext->mergeDrawIntoRender();
				renderContext->direct< EndPassRenderBlock >();
				T_PASS_PROFILE_END();

				if (currentTarget && currentTarget->doubleBuffered)
					currentTarget->targetSet->swap();

				currentTarget = nullptr;
				currentOutput = RenderPass::Output();
			}
			else if (T_PASS_IS_PROFILING)
			{
				// Profiling of a non-output pass.
				T_PASS_PROFILE_BEGIN();
				renderContext->mergeComputeIntoRender();
				renderContext->mergeDrawIntoRender();
				T_PASS_PROFILE_END();
			}

			// Make the graphics queue wait upon asynchronous compute work which this, and
			// thus any later, pass consumes. Recorded outside of any render pass since
			// synchronization can split the graphics queue. The begin pass block above is
			// still pending in the draw queue so the wait gets recorded before it.
			if (syncPoint)
			{
				for (const auto producerIndex : syncBefore->second)
				{
					T_FATAL_ASSERT(asyncComputeHandles[producerIndex] != nullptr);
					renderContext->direct< WaitComputeRenderBlock >(asyncComputeHandles[producerIndex]);
				}
			}

			// Build this pass.
			T_PROFILER_BEGIN(L"RenderGraph build \"" + pass->getName() + L"\"");
			m_buildingPasses = true;

#if !defined(__ANDROID__) && !defined(__IOS__)
			if (m_profiler && !asyncCompute)
			{
				T_FATAL_ASSERT(profiling == nullptr);
				profiling = &passQueryHandles[index];
			}
#endif

			if (asyncCompute)
			{
				// Pass consumes results of earlier asynchronous passes; insert barriers on
				// the compute queue covering compute and acceleration structure writes
				// against compute and acceleration structure reads.
				if (m_barrierBefore.find(index) != m_barrierBefore.end())
				{
					renderContext->compute< BarrierRenderBlock >(Stage::Compute, Stage::Compute, nullptr, 0, true);
					renderContext->compute< BarrierRenderBlock >(Stage::Compute, Stage::AccelerationStructureUpdate, nullptr, 0, true);
					renderContext->compute< BarrierRenderBlock >(Stage::AccelerationStructureUpdate, Stage::AccelerationStructureUpdate | Stage::Compute, nullptr, 0, true);
				}
				renderContext->beginAsyncCompute();
			}

			for (const auto& build : pass->getBuilds())
			{
				build(*this, renderContext);

				// Merge all pending priority draws (sorted by depth) after each build step.
				renderContext->mergePriorityIntoDraw(RenderPriority::All);
			}

			if (asyncCompute)
			{
				renderContext->endAsyncCompute();

				// Fence the asynchronous work with a handle so consumer passes can wait
				// upon it; merged into the render queue immediately so the work is
				// recorded, and possibly submitted, as early as possible.
				ComputeHandle* handle = renderContext->alloc< ComputeHandle >();
				asyncComputeHandles[index] = handle;
				renderContext->compute< SignalComputeRenderBlock >(handle);

#if !defined(__ANDROID__) && !defined(__IOS__)
				// The timestamps only bracket the graphics queue while the asynchronous
				// work is recorded; not the execution of the work itself.
				if (m_profiler)
					renderContext->direct< ProfileBeginRenderBlock >(&passQueryHandles[index]);
#endif
				renderContext->mergeComputeIntoRender();
#if !defined(__ANDROID__) && !defined(__IOS__)
				if (m_profiler)
					renderContext->direct< ProfileEndRenderBlock >(&passQueryHandles[index]);
#endif

				// No consumer pass known; conservatively make all subsequent graphics work
				// observe the result.
				if (waitAfter)
					renderContext->direct< WaitComputeRenderBlock >(handle);
			}

			m_buildingPasses = false;
			T_PROFILER_END();

			// Decrement reference counts on input targets; release if last reference.
			for (const auto& input : inputs)
			{
				if (input.resourceId == 0)
					continue;

				auto it = m_targets.find(RGTargetSet(input.resourceId));
				if (it == m_targets.end())
					continue;

				auto& target = it->second;
				if (--target.inputRefCount <= 0)
				{
					if (!target.external && target.persistentHandle == 0)
						m_context->getTargetSetPool()->release(target.targetSet);
				}
			}
		}
	}

	if (currentOutput.resourceId != ~0U)
	{
		T_PASS_PROFILE_BEGIN();
		renderContext->mergeComputeIntoRender();
		renderContext->mergeDrawIntoRender();
		renderContext->direct< EndPassRenderBlock >();
		T_PASS_PROFILE_END();

		if (currentTarget && currentTarget->doubleBuffered)
			currentTarget->targetSet->swap();
	}

#if !defined(__ANDROID__) && !defined(__IOS__)
	if (m_profiler)
	{
		renderContext->direct< ProfileEndRenderBlock >(referenceQueryHandle);

		// Report all queries last using reference query to calculate offset.
		int32_t ordinal = 0;
		for (int32_t i = sizeof_array(m_order) - 1; i >= 0; --i)
		{
			const auto& order = m_order[i];
			for (int32_t j = 0; j < (int32_t)order.size(); ++j)
			{
				const uint32_t index = order[j];
				const auto pass = m_passes[index];

				auto pr = renderContext->alloc< ProfileReportRenderBlock >();
				pr->queryHandle = &passQueryHandles[index];
				pr->referenceQueryHandle = referenceQueryHandle;
				pr->offset = referenceOffset;
				pr->sink = [=, name = pass->getName(), this](double start, double duration) {
					m_profiler(ordinal, i, name, start, duration);
				};
				renderContext->direct(pr);

				++ordinal;
			}
		}
	}
#endif

	T_FATAL_ASSERT(!renderContext->havePendingComputes());
	T_FATAL_ASSERT(!renderContext->havePendingDraws());

	// Ensure all persistent targets are returned to pool, since we're
	// manually acquiring all at the beginning.
	for (auto& it : m_targets)
	{
		auto& target = it.second;
		if (target.persistentHandle != 0)
			m_context->getTargetSetPool()->release(target.targetSet);
		target.realized = { 0, 0 };
		target.targetSet = nullptr;
	}

	for (auto& it : m_buffers)
	{
		auto& sbuffer = it.second;
		if (sbuffer.buffer != nullptr)
			m_context->getBufferPool()->release(sbuffer.buffer);
	}

	for (auto& it : m_textures)
	{
		auto& texture = it.second;
		if (texture.texture != nullptr)
			m_context->getTexturePool()->release(texture.texture);
	}

	// Cleanup pool data structures.
	m_context->cleanup();

	// Remove all data; keeps memory allocated for arrays
	// since it's very likely this will be identically
	// re-populated next frame.
	cleanup();

	m_counter++;
	return true;
}

bool RenderGraph::realizeTargetDimensions(int32_t width, int32_t height, RGTargetSet targetId)
{
	TargetResource& target = m_targets[targetId];
	if (target.realized.width != 0 && target.realized.height != 0)
		return true;

	const RenderGraphTargetSetDesc& targetSetDesc = target.targetSetDesc;
	if (targetSetDesc.width != 0 && targetSetDesc.height != 0)
	{
		width = targetSetDesc.width;
		height = targetSetDesc.height;
	}

	if (isExplicitTargetSetId(target.sizeReferenceTargetSetId))
	{
		if (!realizeTargetDimensions(width, height, target.sizeReferenceTargetSetId))
			return false;

		const TargetResource* tr = findTargetResource(m_targets, target.sizeReferenceTargetSetId);
		T_FATAL_ASSERT(tr != nullptr);

		width = tr->realized.width;
		height = tr->realized.height;
	}

	if (isExplicitTargetSetId(target.sharedDepthStencilTargetSetId))
	{
		if (!realizeTargetDimensions(width, height, target.sharedDepthStencilTargetSetId))
			return false;

		const TargetResource* tr = findTargetResource(m_targets, target.sharedDepthStencilTargetSetId);
		T_FATAL_ASSERT(tr != nullptr);

		width = tr->realized.width;
		height = tr->realized.height;
	}

	if (targetSetDesc.referenceWidthDenom > 0)
		width = (width * targetSetDesc.referenceWidthMul + targetSetDesc.referenceWidthDenom - 1) / targetSetDesc.referenceWidthDenom;
	if (targetSetDesc.referenceHeightDenom > 0)
		height = (height * targetSetDesc.referenceHeightMul + targetSetDesc.referenceHeightDenom - 1) / targetSetDesc.referenceHeightDenom;

	if (targetSetDesc.maxWidth > 0)
		width = min< int32_t >(width, targetSetDesc.maxWidth);
	if (targetSetDesc.maxHeight > 0)
		height = min< int32_t >(height, targetSetDesc.maxHeight);

	target.realized = { width, height };
	return true;
}

bool RenderGraph::acquire(TargetResource& inoutTarget)
{
	RGTargetSet sharedDepthStencilTargetSetId = inoutTarget.sharedDepthStencilTargetSetId;

	if (isExplicitTargetSetId(sharedDepthStencilTargetSetId))
	{
		// This target depends on depth buffer from non-primary target; search chain for
		// actual depth buffer.
		for (;;)
		{
			const TargetResource* tr = findTargetResource(m_targets, sharedDepthStencilTargetSetId);
			T_FATAL_ASSERT(tr != nullptr);

			// Do this target have a depth buffer?
			if (tr->targetSetDesc.createDepthStencil)
				break;

			sharedDepthStencilTargetSetId = tr->sharedDepthStencilTargetSetId;

			// Do this target use primary depth buffer?
			if (tr->sharedDepthStencilTargetSetId == RGTargetSet::Output)
				break;

			// If this target not have a shared depth valid then it's an error.
			if (tr->sharedDepthStencilTargetSetId == RGTargetSet::Invalid)
				return false;
		}
	}

	Ref< IRenderTargetSet > sharedDepthTargetSet;
	if (isExplicitTargetSetId(sharedDepthStencilTargetSetId))
	{
		const TargetResource* tr = findTargetResource(m_targets, sharedDepthStencilTargetSetId);
		T_FATAL_ASSERT(tr != nullptr);
		sharedDepthTargetSet = tr->targetSet->getWriteTargetSet();
	}

	const bool sharedPrimaryDepthStencilTargetSet = (sharedDepthStencilTargetSetId == RGTargetSet::Output);

	inoutTarget.targetSet = m_context->getTargetSetPool()->acquire(
		inoutTarget.name,
		inoutTarget.targetSetDesc,
		sharedDepthTargetSet,
		sharedPrimaryDepthStencilTargetSet,
		inoutTarget.realized.width,
		inoutTarget.realized.height,
		m_multiSample,
		inoutTarget.doubleBuffered,
		inoutTarget.persistentHandle);

	if (!inoutTarget.targetSet)
		return false;

	return true;
}

void RenderGraph::cleanup()
{
	m_passes.resize(0);
	m_targets.reset();
	m_buffers.reset();
	m_textures.reset();
	for (int32_t i = 0; i < sizeof_array(m_order); ++i)
		m_order[i].resize(0);
	m_sharedDepthTargets.clear();
	m_dependencies.reset();
	m_syncBefore.reset();
	m_barrierBefore.reset();
	m_waitAfter.reset();
	m_nextResourceId = 1;
}

}
