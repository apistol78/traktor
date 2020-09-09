#include "Core/Containers/StaticSet.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Timer/Profiler.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Frame/RenderGraphTargetSetPool.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

typedef StaticSet< uint32_t, 512 > visitedSet_t;

void traverse(const RefArray< const RenderPass >& passes, int32_t index, visitedSet_t& visited, const std::function< void(int32_t) >& fn)
{
	if (!visited.insert(index))
		return;

	// Traverse inputs first as we want to traverse passes depth-first.
	for (const auto& input : passes[index]->getInputs())
	{
		for (int32_t i = 0; i < passes.size(); ++i)
		{
			if (i != index && passes[i]->getOutput().targetSetId == input.targetSetId)
				traverse(passes, i, visited, fn);
		}
	}

	// Call visitor for this pass.
	fn(index);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderGraph", RenderGraph, Object)

RenderGraph::RenderGraph(IRenderSystem* renderSystem)
:	m_pool(new RenderGraphTargetSetPool(renderSystem))
,	m_nextTargetSetId(1)
{
}

RenderGraph::~RenderGraph()
{
	T_FATAL_ASSERT_M(m_pool == nullptr, L"Forgot to destroy RenderGraph instance.");
}

void RenderGraph::destroy()
{
	m_targets.clear();
	m_passes.clear();
	m_order.clear();

	if (m_pool != nullptr)
	{
		delete m_pool;
		m_pool = nullptr;
	}
}

handle_t RenderGraph::addTransientTargetSet(
	const wchar_t* const name,
	const RenderGraphTargetSetDesc& targetSetDesc,
	IRenderTargetSet* sharedDepthStencilTargetSet,
	handle_t sizeReferenceTargetSetId
)
{
	handle_t targetSetId = m_nextTargetSetId++;

	auto& target = m_targets[targetSetId];
	target.name = name;
	target.persistentHandle = 0;
	target.targetSetDesc = targetSetDesc;
	target.sharedDepthStencilTargetSet = sharedDepthStencilTargetSet;
	target.sizeReferenceTargetSetId = sizeReferenceTargetSetId;
	target.referenceCount = 0;
	target.external = false;

	return targetSetId;
}

handle_t RenderGraph::addPersistentTargetSet(
	const wchar_t* const name,
	handle_t persistentHandle,
	const RenderGraphTargetSetDesc& targetSetDesc,
	IRenderTargetSet* sharedDepthStencilTargetSet,
	handle_t sizeReferenceTargetSetId
)
{
	handle_t targetSetId = m_nextTargetSetId++;

	auto& target = m_targets[targetSetId];
	target.name = name;
	target.persistentHandle = persistentHandle;
	target.targetSetDesc = targetSetDesc;
	target.sharedDepthStencilTargetSet = sharedDepthStencilTargetSet;
	target.sizeReferenceTargetSetId = sizeReferenceTargetSetId;
	target.referenceCount = 0;
	target.external = false;

	return targetSetId;
}

handle_t RenderGraph::addExternalTargetSet(const wchar_t* const name, IRenderTargetSet* targetSet)
{
	T_FATAL_ASSERT(targetSet != nullptr);
	T_ASSERT(targetSet->getWidth() > 0);
	T_ASSERT(targetSet->getHeight() > 0);

	handle_t targetSetId = m_nextTargetSetId++;

	auto& target = m_targets[targetSetId];
	target.name = name;
	target.persistentHandle = 0;
	target.rts = targetSet;
	target.sizeReferenceTargetSetId = 0;
	target.referenceCount = 0;
	target.external = true;

	return targetSetId;
}

handle_t RenderGraph::findTargetByName(const wchar_t* const name) const
{
	for (const auto& tm : m_targets)
	{
		if (wcscmp(tm.second.name, name) == 0)
			return tm.first;
	}
	return 0;
}

IRenderTargetSet* RenderGraph::getTargetSet(handle_t targetSetId) const
{
	auto it = m_targets.find(targetSetId);
	if (it != m_targets.end())
		return it->second.rts;
	else
		return nullptr;
}

void RenderGraph::addPass(const RenderPass* pass)
{
	m_passes.push_back(pass);
}

bool RenderGraph::validate()
{
	visitedSet_t added;

	// Collect order of passes, depth-first.
	m_order.resize(0);	
	for (int32_t i = 0; i < (int32_t)m_passes.size(); ++i)
	{
		if (m_passes[i]->haveOutput())
		{
			const auto& output = m_passes[i]->getOutput();
			if (output.targetSetId == 0)
			{
				// "Render to primary" passes is included as roots.
				traverse(m_passes, i, added, [&](int32_t index) {
					m_order.push_back(index);
				});
			}
			else
			{
				// External outputs indicate some sort of caching scheme, such
				// as offscreen caching etc, so we need to ensure they are executed
				// regardless of dependencies.
				auto it = m_targets.find(output.targetSetId);
				if (it->second.external)
				{
					traverse(m_passes, i, added, [&](int32_t index) {
						m_order.push_back(index);
					});
				}
			}
		}
		else
		{
			// Passes which doesn't have an output need to be included as roots.
			traverse(m_passes, i, added, [&](int32_t index) {
				m_order.push_back(index);
			});
		}
	}

	// Calculate target reference counts,
	// also set storage flags of each pass's output.
	for (auto index : m_order)
	{
		const auto pass = m_passes[index];
		for (const auto& input : pass->getInputs())
		{
			if (input.targetSetId == 0)
				continue;

			auto it = m_targets.find(input.targetSetId);
			if (it == m_targets.end())
				return false;

			auto& target = it->second;
			target.referenceCount++;
		}
	}

	return true;
}

bool RenderGraph::build(RenderContext* renderContext, int32_t width, int32_t height)
{
	T_FATAL_ASSERT(!renderContext->havePendingDraws());

	// Acquire all persistent targets first in case they are read from
	// before being used as an output.
	for (auto& it : m_targets)
	{
		auto& target = it.second;
		if (target.rts == nullptr && target.persistentHandle != 0)
		{
			T_ASSERT(target.referenceCount > 0);
			if (!acquire(width, height, target))
				return false;
		}
	}

	double referenceOffset = Profiler::getInstance().getTime();

	// Allocate query handles from render context's heap.
	int32_t* queryHandles = (int32_t*)renderContext->alloc((m_passes.size() + 1) * sizeof(int32_t), alignOf< int32_t >());
	int32_t* referenceQueryHandle = queryHandles;
	int32_t* passQueryHandles = queryHandles + 1;

	auto pb = renderContext->alloc< ProfileBeginRenderBlock >();
	pb->queryHandle = referenceQueryHandle;
	renderContext->enqueue(pb);

	// Render passes in dependency order.
	for (auto index : m_order)
	{
		const auto pass = m_passes[index];
		const auto& inputs = pass->getInputs();
		const auto& output = pass->getOutput();

		// Begin render pass.
		if (pass->haveOutput())
		{
			if (output.targetSetId != 0)
			{
				auto it = m_targets.find(output.targetSetId);
				T_FATAL_ASSERT(it != m_targets.end());

				auto& target = it->second;
				if (target.rts == nullptr)
				{
					T_ASSERT(!target.external);
					if (!acquire(width, height, target))
						return false;
				}	

				auto tb = renderContext->alloc< BeginPassRenderBlock >(pass->getName());
				tb->renderTargetSet = target.rts;
				tb->clear = output.clear;
				tb->load = output.load;
				tb->store = output.store;
				renderContext->enqueue(tb);
			}
			else
			{
				auto tb = renderContext->alloc< BeginPassRenderBlock >(pass->getName());
				tb->clear = output.clear;
				tb->load = output.load;
				tb->store = output.store;
				renderContext->enqueue(tb);			
			}
		}

		// Alloc query handle; are freed after being reported.
		auto pb = renderContext->alloc< ProfileBeginRenderBlock >();
		pb->queryHandle = &passQueryHandles[index];
		renderContext->enqueue(pb);

		// Build this pass.
		for (const auto& build : pass->getBuilds())
		{
			build(*this, renderContext);
			T_FATAL_ASSERT(!renderContext->havePendingDraws());
		}

		// End render pass.
		if (pass->haveOutput())
		{
			auto te = renderContext->alloc< EndPassRenderBlock >();
			renderContext->enqueue(te);
		}

		auto pe = renderContext->alloc< ProfileEndRenderBlock >();
		pe->queryHandle = &passQueryHandles[index];
		renderContext->enqueue(pe);

		// Decrement reference counts on input targets; release if last reference.
		for (const auto& input : inputs)
		{
			if (input.targetSetId == 0)
				continue;

			auto it = m_targets.find(input.targetSetId);
			if (it == m_targets.end())
				return false;

			auto& target = it->second;
			if (--target.referenceCount <= 0)
			{
				if (!target.external)
					m_pool->release(target.rts);
				target.rts = nullptr;
			}
		}
	}

	T_FATAL_ASSERT(!renderContext->havePendingDraws());

	auto pe = renderContext->alloc< ProfileEndRenderBlock >();
	pe->queryHandle = referenceQueryHandle;
	renderContext->enqueue(pe);

	// Report all queries last using reference query to calculate offset.
	for (auto index : m_order)
	{
		const auto pass = m_passes[index];
		auto pr = renderContext->alloc< ProfileReportRenderBlock >();
		pr->name = pass->getName();
		pr->queryHandle = &passQueryHandles[index];
		pr->referenceQueryHandle = referenceQueryHandle;
		pr->offset = referenceOffset;
		renderContext->enqueue(pr);
	}

	// Cleanup pool data structure.
	m_pool->cleanup();

	// Remove all data; keep memory allocated for arrays
	// since it's very likely this will be identically
	// re-populated next frame.
	m_targets.reset();
	m_passes.resize(0);
	m_order.resize(0);
	return true;
}

bool RenderGraph::acquire(int32_t width, int32_t height, Target& outTarget)
{
	// Use size of reference target.
	if (outTarget.sizeReferenceTargetSetId != 0)
	{
		T_ASSERT_M(outTarget.sharedDepthStencilTargetSet == nullptr, L"Cannot use both reference target and shared depth/stencil target for size reference.");

		auto it = m_targets.find(outTarget.sizeReferenceTargetSetId);
		if (it == m_targets.end())
			return false;

		width = it->second.targetSetDesc.width;
		height = it->second.targetSetDesc.height;
	}

	// Use size of shared depth/stencil target since they must match.
	if (outTarget.sharedDepthStencilTargetSet != nullptr)
	{
		T_ASSERT_M(outTarget.sizeReferenceTargetSetId == 0, L"Cannot use both reference target and shared depth/stencil target for size reference.");

		width = outTarget.sharedDepthStencilTargetSet->getWidth();
		height = outTarget.sharedDepthStencilTargetSet->getHeight();
	}

	outTarget.rts = m_pool->acquire(
		outTarget.name,
		outTarget.targetSetDesc,
		outTarget.sharedDepthStencilTargetSet,
		width,
		height,
		outTarget.persistentHandle
	);
	if (!outTarget.rts)
		return false;

	return true;
}

	}
}