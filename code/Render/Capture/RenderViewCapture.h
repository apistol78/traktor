#ifndef traktor_render_RenderViewCapture_H
#define traktor_render_RenderViewCapture_H

#include "Render/IRenderView.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace render
	{

class IRenderSystem;
class RenderTargetSet;

/*! \brief Performance capture render view.
 * \ingroup RenderCapture
 */
class RenderViewCapture : public IRenderView
{
	T_RTTI_CLASS;

public:
	RenderViewCapture(IRenderSystem* renderSystem, IRenderView* renderView);

	virtual bool nextEvent(RenderEvent& outEvent);

	virtual void close();

	virtual bool reset(const RenderViewDefaultDesc& desc);

	virtual bool reset(int32_t width, int32_t height);

	virtual int getWidth() const;

	virtual int getHeight() const;

	virtual bool isActive() const;

	virtual bool isFullScreen() const;

	virtual void showCursor();

	virtual void hideCursor();

	virtual bool isCursorVisible() const;

	virtual bool setGamma(float gamma);

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual SystemWindow getSystemWindow();

	virtual bool begin(EyeType eye);

	virtual bool begin(RenderTargetSet* renderTargetSet);

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget);

	virtual void clear(uint32_t clearMask, const Color4f* color, float depth, int32_t stencil);

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives);

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount);

	virtual void end();

	virtual void present();

	virtual void pushMarker(const char* const marker);

	virtual void popMarker();

	virtual void getStatistics(RenderViewStatistics& outStatistics) const;

	virtual bool getBackBufferContent(void* buffer) const;

private:
	Ref< IRenderSystem > m_renderSystem;
	Ref< IRenderView > m_renderView;
	bool m_captureFrame;
	Ref< RenderTargetSet > m_captureTarget;
	Ref< drawing::Image > m_captureImage;
	int32_t m_captureDepth;
	uint32_t m_drawCount;
	uint32_t m_frameCount;
};

	}
}

#endif	// traktor_render_RenderViewCapture_H
