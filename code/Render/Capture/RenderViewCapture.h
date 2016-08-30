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

	virtual bool nextEvent(RenderEvent& outEvent) T_OVERRIDE T_FINAL;

	virtual void close() T_OVERRIDE T_FINAL;

	virtual bool reset(const RenderViewDefaultDesc& desc) T_OVERRIDE T_FINAL;

	virtual bool reset(int32_t width, int32_t height) T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;

	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual bool isActive() const T_OVERRIDE T_FINAL;

	virtual bool isMinimized() const T_OVERRIDE T_FINAL;

	virtual bool isFullScreen() const T_OVERRIDE T_FINAL;

	virtual void showCursor() T_OVERRIDE T_FINAL;

	virtual void hideCursor() T_OVERRIDE T_FINAL;

	virtual bool isCursorVisible() const T_OVERRIDE T_FINAL;

	virtual bool setGamma(float gamma) T_OVERRIDE T_FINAL;

	virtual void setViewport(const Viewport& viewport) T_OVERRIDE T_FINAL;

	virtual Viewport getViewport() T_OVERRIDE T_FINAL;

	virtual SystemWindow getSystemWindow() T_OVERRIDE T_FINAL;

	virtual bool begin(EyeType eye) T_OVERRIDE T_FINAL;

	virtual bool begin(RenderTargetSet* renderTargetSet) T_OVERRIDE T_FINAL;

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget) T_OVERRIDE T_FINAL;

	virtual void clear(uint32_t clearMask, const Color4f* color, float depth, int32_t stencil) T_OVERRIDE T_FINAL;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives) T_OVERRIDE T_FINAL;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount) T_OVERRIDE T_FINAL;

	virtual void end() T_OVERRIDE T_FINAL;

	virtual void present() T_OVERRIDE T_FINAL;

	virtual void pushMarker(const char* const marker) T_OVERRIDE T_FINAL;

	virtual void popMarker() T_OVERRIDE T_FINAL;

	virtual void getStatistics(RenderViewStatistics& outStatistics) const T_OVERRIDE T_FINAL;

	virtual bool getBackBufferContent(void* buffer) const T_OVERRIDE T_FINAL;

private:
	struct ProfileCapture
	{
		const wchar_t* name;
		intptr_t begin;
		intptr_t end;
	};

	Ref< IRenderSystem > m_renderSystem;
	Ref< IRenderView > m_renderView;
	Ref< ITimeQuery > m_timeQuery;
	int32_t m_targetDepth;
	std::vector< ProfileCapture > m_timeStamps;
};

	}
}

#endif	// traktor_render_RenderViewCapture_H
