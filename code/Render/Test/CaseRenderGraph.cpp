/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Test/CaseRenderGraph.h"

#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/String.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderView.h"

#include <utility>

namespace traktor::render::test
{
namespace
{

/*! Render view which records work and time queries in order issued. */
class RenderViewRecord : public IRenderView
{
public:
	void record(const std::wstring& event)
	{
		m_log += (m_log.empty() ? L"" : L" ") + event;
		++m_clock;
	}

	const std::wstring& getLog() const { return m_log; }

	virtual bool nextEvent(RenderEvent& outEvent) override final { return false; }

	virtual void close() override final {}

	virtual bool reset(const RenderViewDefaultDesc& desc) override final { return true; }

	virtual bool reset(int32_t width, int32_t height) override final { return true; }

	virtual uint32_t getDisplay() const override final { return 0; }

	virtual int getWidth() const override final { return 64; }

	virtual int getHeight() const override final { return 64; }

	virtual bool isActive() const override final { return true; }

	virtual bool isMinimized() const override final { return false; }

	virtual bool isFullScreen() const override final { return false; }

	virtual void showCursor() override final {}

	virtual void hideCursor() override final {}

	virtual bool isCursorVisible() const override final { return true; }

	virtual bool isHDR() const override final { return false; }

	virtual void setViewport(const Viewport& viewport) override final {}

	virtual void setScissor(const Rectangle& scissor) override final {}

	virtual SystemWindow getSystemWindow() override final { return SystemWindow(); }

	virtual bool beginFrame() override final { return true; }

	virtual void endFrame() override final {}

	virtual void present() override final {}

	virtual bool beginPass(const Clear* clear, uint32_t load, uint32_t store) override final
	{
		record(L"beginPass");
		return true;
	}

	virtual bool beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear, uint32_t load, uint32_t store) override final
	{
		record(L"beginPass");
		return true;
	}

	virtual bool beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear, uint32_t load, uint32_t store) override final
	{
		record(L"beginPass");
		return true;
	}

	virtual void endPass() override final { record(L"endPass"); }

	virtual void clear(const Clear* clear, const Rectangle& rectangle) override final {}

	virtual void draw(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, IProgram* program, const Primitives& primitives, uint32_t instanceCount) override final {}

	virtual void drawIndirect(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, IProgram* program, PrimitiveType primitiveType, const IBufferView* drawBuffer, uint32_t drawOffset, uint32_t drawCount) override final {}

	virtual void compute(IProgram* program, const int32_t* workSize, bool asynchronous) override final {}

	virtual void computeIndirect(IProgram* program, const IBufferView* workBuffer, uint32_t workOffset) override final {}

	virtual void barrier(Stage from, Stage to, ITexture* written, uint32_t writtenMip, bool asynchronous) override final {}

	virtual void synchronize() override final {}

	virtual ComputeHandle signalAsynchronousCompute() override final
	{
		record(L"signal");
		return { 1 };
	}

	virtual void waitAsynchronousCompute(ComputeHandle handle) override final { record(L"wait"); }

	virtual bool copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion) override final { return true; }

	virtual void writeAccelerationStructure(IAccelerationStructure* accelerationStructure, const AlignedVector< IAccelerationStructure::Instance >& instances, bool asynchronous) override final {}

	virtual void writeAccelerationStructure(IAccelerationStructure* accelerationStructure, const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, const AlignedVector< RaytracingPrimitives >& primitives, bool rebuild, bool asynchronous) override final {}

	virtual int32_t beginTimeQuery(bool asynchronous) override final
	{
		const int32_t query = (int32_t)m_stamps.size() * 2;
		record(L"begin" + toString(query) + (asynchronous ? L"a" : L""));
		m_stamps.push_back({ m_clock, -1 });
		return query;
	}

	virtual void endTimeQuery(int32_t query, bool asynchronous) override final
	{
		record(L"end" + toString(query) + (asynchronous ? L"a" : L""));
		m_stamps[query / 2].second = m_clock;
	}

	virtual bool getTimeQuery(int32_t query, bool wait, double& outStart, double& outEnd) const override final
	{
		if (query < 0 || query / 2 >= (int32_t)m_stamps.size() || m_stamps[query / 2].second < 0)
			return false;
		outStart = (double)m_stamps[query / 2].first;
		outEnd = (double)m_stamps[query / 2].second;
		return true;
	}

	virtual void pushMarker(bool asynchronous, const std::wstring& marker) override final {}

	virtual void popMarker(bool asynchronous) override final {}

	virtual void writeMarker(bool asynchronous, const std::wstring& marker) override final {}

	virtual void getStatistics(RenderViewStatistics& outStatistics) const override final {}

private:
	std::wstring m_log;
	int32_t m_clock = 0;
	AlignedVector< std::pair< int32_t, int32_t > > m_stamps;
};

void record(RenderContext* renderContext, const wchar_t* event, bool draw)
{
	auto rb = renderContext->alloc< LambdaRenderBlock >([=](IRenderView* renderView) {
		static_cast< RenderViewRecord* >(renderView)->record(event);
	});
	if (draw)
		renderContext->draw(rb);
	else
		renderContext->compute(rb);
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.test.CaseRenderGraph", 0, CaseRenderGraph, traktor::test::Case)

void CaseRenderGraph::run()
{
	Ref< RenderGraph > rg = new RenderGraph((IRenderSystem*)nullptr, 0);

	RenderGraphTargetSetDesc desc;
	desc.count = 1;
	desc.width = 64;
	desc.height = 64;
	desc.targets[0].colorFormat = TfR8G8B8A8;
	const RGTargetSet target = rg->addTransientTargetSet(L"Target", desc);

	{
		Ref< RenderPass > rp = new RenderPass(L"First");
		rp->addInput(target);
		rp->setOutput(target, render::TfAll, render::TfAll);
		rg->addPass(rp);
	}

	{
		Ref< RenderPass > rp = new RenderPass(L"Second");
		rp->addInput(target);
		rp->setOutput(RGTargetSet::Output, render::TfAll, render::TfAll);
		rg->addPass(rp);
	}

	bool result = rg->validate();

	rg->destroy();
	rg = nullptr;

	CASE_ASSERT(result);

#if !defined(__ANDROID__) && !defined(__IOS__)
	struct Report
	{
		RenderPass::Queue queue;
		std::wstring name;
		double duration;
	};

	AlignedVector< Report > reports;
	const RenderGraph::fn_profiler_t profiler = [&](int32_t pass, int32_t level, RenderPass::Queue queue, const std::wstring& name, double start, double duration) {
		reports.push_back({ queue, name, duration });
	};

	Ref< RenderContext > renderContext = new RenderContext(1024 * 1024);

	// Asynchronous compute pass is measured on the asynchronous compute queue, and it doesn't
	// include synchronous compute of the pending graphics pass recorded before it.
	{
		Ref< RenderViewRecord > renderView = new RenderViewRecord();

		rg = new RenderGraph((IRenderSystem*)nullptr, 0, profiler);
		const RGDependency dependency = rg->addDependency();

		Ref< RenderPass > rp = new RenderPass(L"Sync");
		rp->addInput(RGDependency::First);
		rp->addBuild([](const RenderGraph&, RenderContext* renderContext) {
			record(renderContext, L"sync", false);
		});
		rg->addPass(rp);

		rp = new RenderPass(L"Async", RenderPass::Queue::AsyncCompute);
		rp->addInput(RGDependency::First);
		rp->setOutput(dependency);
		rp->addBuild([](const RenderGraph&, RenderContext* renderContext) {
			record(renderContext, L"async", false);
		});
		rg->addPass(rp);

		rp = new RenderPass(L"Consumer");
		rp->addInput(dependency);
		rp->setOutput(RGTargetSet::Output, render::TfAll, render::TfAll);
		rp->addBuild([](const RenderGraph&, RenderContext* renderContext) {
			record(renderContext, L"draw", true);
		});
		rg->addPass(rp);

		CASE_ASSERT(rg->validate());
		CASE_ASSERT(rg->build(renderContext, 64, 64));
		renderContext->render(renderView);
		renderContext->flush();

		CASE_ASSERT_EQUAL(renderView->getLog(), std::wstring(L"begin0 begin2a async end2a signal begin4 sync end4 wait begin6 beginPass draw endPass end6 end0"));
		CASE_ASSERT_EQUAL((int32_t)reports.size(), 3);
		if (reports.size() == 3)
		{
			CASE_ASSERT_EQUAL(reports[0].name, std::wstring(L"Sync"));
			CASE_ASSERT(reports[0].queue == RenderPass::Queue::Graphics);
			CASE_ASSERT_EQUAL(reports[0].duration, 2.0);
			CASE_ASSERT_EQUAL(reports[1].name, std::wstring(L"Async"));
			CASE_ASSERT(reports[1].queue == RenderPass::Queue::AsyncCompute);
			CASE_ASSERT_EQUAL(reports[1].duration, 2.0);
			CASE_ASSERT_EQUAL(reports[2].name, std::wstring(L"Consumer"));
			CASE_ASSERT(reports[2].queue == RenderPass::Queue::Graphics);
			CASE_ASSERT_EQUAL(reports[2].duration, 4.0);
		}

		rg->destroy();
		rg = nullptr;
		reports.resize(0);
	}

	// Pending work of last pass, which doesn't render into a target, is flushed and measured.
	{
		Ref< RenderViewRecord > renderView = new RenderViewRecord();

		rg = new RenderGraph((IRenderSystem*)nullptr, 0, profiler);

		Ref< RenderPass > rp = new RenderPass(L"Draw");
		rp->setOutput(RGTargetSet::Output, render::TfAll, render::TfAll);
		rp->addBuild([](const RenderGraph&, RenderContext* renderContext) {
			record(renderContext, L"draw", true);
		});
		rg->addPass(rp);

		rp = new RenderPass(L"Last");
		rp->addInput(RGDependency::Last);
		rp->addBuild([](const RenderGraph&, RenderContext* renderContext) {
			record(renderContext, L"compute", false);
		});
		rg->addPass(rp);

		CASE_ASSERT(rg->validate());
		CASE_ASSERT(rg->build(renderContext, 64, 64));
		renderContext->render(renderView);
		renderContext->flush();

		CASE_ASSERT_EQUAL(renderView->getLog(), std::wstring(L"begin0 begin2 beginPass draw endPass end2 begin4 compute end4 end0"));
		CASE_ASSERT_EQUAL((int32_t)reports.size(), 2);
		if (reports.size() == 2)
		{
			CASE_ASSERT_EQUAL(reports[1].name, std::wstring(L"Last"));
			CASE_ASSERT_EQUAL(reports[1].duration, 2.0);
		}

		rg->destroy();
		rg = nullptr;
		reports.resize(0);
	}

	// Nothing is measured when profiler is disabled.
	{
		Ref< RenderViewRecord > renderView = new RenderViewRecord();

		rg = new RenderGraph((IRenderSystem*)nullptr, 0, profiler);
		rg->setProfilerEnable(false);

		Ref< RenderPass > rp = new RenderPass(L"Draw");
		rp->setOutput(RGTargetSet::Output, render::TfAll, render::TfAll);
		rp->addBuild([](const RenderGraph&, RenderContext* renderContext) {
			record(renderContext, L"draw", true);
		});
		rg->addPass(rp);

		CASE_ASSERT(rg->validate());
		CASE_ASSERT(rg->build(renderContext, 64, 64));
		renderContext->render(renderView);
		renderContext->flush();

		CASE_ASSERT_EQUAL(renderView->getLog(), std::wstring(L"beginPass draw endPass"));
		CASE_ASSERT_EQUAL((int32_t)reports.size(), 0);

		rg->destroy();
		rg = nullptr;
	}
#endif
}

}
