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

	virtual Viewport getViewport() override final;

	virtual SystemWindow getSystemWindow() override final;

	virtual bool begin(const Clear* clear) override final;

	virtual bool begin(IRenderTargetSet* renderTargetSet, const Clear* clear) override final;

	virtual bool begin(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear) override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives) override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount) override final;

	virtual void compute(IProgram* program, const int32_t* workSize) override final;

	virtual bool copy(ITexture* destinationTexture, int32_t destinationSide, int32_t destinationLevel, ITexture* sourceTexture, int32_t sourceSide, int32_t sourceLevel) override final;

	virtual void end() override final;

	virtual void flush() override final;

	virtual void present() override final;

	virtual void pushMarker(const char* const marker) override final;

	virtual void popMarker() override final;

	virtual void getStatistics(RenderViewStatistics& outStatistics) const override final;

	virtual bool getBackBufferContent(void* buffer) const override final;

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
