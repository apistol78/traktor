#ifndef traktor_render_RenderViewCapture_H
#define traktor_render_RenderViewCapture_H

#include "Render/IRenderView.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_CAPTURE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

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
class T_DLLCLASS RenderViewCapture : public IRenderView
{
	T_RTTI_CLASS;

public:
	RenderViewCapture(IRenderSystem* renderSystem, IRenderView* renderView);

	virtual void close();

	virtual void resize(int32_t width, int32_t height);

	virtual int getWidth() const;

	virtual int getHeight() const;

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual bool getNativeAspectRatio(float& outAspectRatio) const;

	virtual bool begin();

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil);

	virtual void clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil);

	virtual void setVertexBuffer(VertexBuffer* vertexBuffer);

	virtual void setIndexBuffer(IndexBuffer* indexBuffer);

	virtual void setProgram(IProgram* program);

	virtual void draw(const Primitives& primitives);

	virtual void end();

	virtual void present();

	virtual void setMSAAEnable(bool msaaEnable);

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
