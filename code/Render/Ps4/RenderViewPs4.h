#pragma once

#include <gnm.h>
#include "Render/IRenderView.h"

namespace traktor
{
	namespace render
	{

class ContextPs4;
class RenderQueuePs4;
class RenderTargetSetPs4;

/*!
 * \ingroup GNM
 */
class RenderViewPs4 : public IRenderView
{
	T_RTTI_CLASS;

public:
	RenderViewPs4(ContextPs4* context);

	virtual ~RenderViewPs4();

	bool create();

	virtual bool nextEvent(RenderEvent& outEvent) override final;

	virtual void close() override final;

	virtual bool reset(const RenderViewDefaultDesc& desc) override final;

	virtual bool reset(int32_t width, int32_t height) override final;

	virtual int getWidth() const override final;

	virtual int getHeight() const override final;

	virtual bool isActive() const override final;

	virtual bool isMinimized() const override final;

	virtual bool isFullScreen() const override final;

	virtual void showCursor() override final;

	virtual void hideCursor() override final;

	virtual bool isCursorVisible() const override final;

	virtual bool setGamma(float gamma) override final;

	virtual void setViewport(const Viewport& viewport) override final;

	virtual SystemWindow getSystemWindow() override final;

	virtual bool beginFrame() override final;

	virtual void endFrame() override final;

	virtual void present() override final;

	virtual bool beginPass(const Clear* clear) override final;

	virtual bool beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear, uint32_t load, uint32_t store) override final;

	virtual bool beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear, uint32_t load, uint32_t store) override final;

	virtual void endPass() override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives) override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount) override final;

	virtual void compute(IProgram* program, const int32_t* workSize) override final;

	virtual bool copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion) override final;

	virtual int32_t beginTimeQuery() override final;

	virtual void endTimeQuery(int32_t query) override final;

	virtual bool getTimeQuery(int32_t query, bool wait, double& outDuration) const override final;

	virtual void pushMarker(const char* const marker) override final;

	virtual void popMarker() override final;

	virtual void getStatistics(RenderViewStatistics& outStatistics) const override final;

private:
	enum { FrameQueueCount = 2 };

	Ref< ContextPs4 > m_context;
	Ref< RenderQueuePs4 > m_queues[FrameQueueCount];
	Ref< RenderTargetSetPs4 > m_primaryTargets[FrameQueueCount];
	SceKernelEqueue m_eopEventQueue;
	int32_t m_videoOutHandle;
	int32_t m_currentQueue;
	int32_t m_width;
	int32_t m_height;
};

	}
}
