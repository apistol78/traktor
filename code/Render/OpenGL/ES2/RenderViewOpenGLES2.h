#ifndef traktor_render_RenderViewOpenGLES2_H
#define traktor_render_RenderViewOpenGLES2_H

#include <list>
#include <stack>
#include "Render/IRenderView.h"
#include "Render/OpenGL/Platform.h"

#if defined(_WIN32)
#	include "Render/OpenGL/ES2/Win32/Window.h"
#endif

namespace traktor
{
	namespace render
	{

#if !defined(T_OFFLINE_ONLY)

class ContextOpenGLES2;
class VertexBufferOpenGLES2;
class IndexBufferOpenGLES2;
class ProgramOpenGLES2;
class RenderTargetSetOpenGLES2;
class RenderTargetOpenGLES2;
class StateCache;

/*!
 * \ingroup OGL
 */
class RenderViewOpenGLES2
:	public IRenderView
#if defined(_WIN32)
,	public IWindowListener
#endif
{
	T_RTTI_CLASS;

public:
	RenderViewOpenGLES2(
		ContextOpenGLES2* globalContext,
		ContextOpenGLES2* context
	);

	virtual ~RenderViewOpenGLES2();
	
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

	virtual void clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil);

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives);

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount);

	virtual void end();

	virtual void present();

	virtual void pushMarker(const char* const marker);

	virtual void popMarker();

	virtual void getStatistics(RenderViewStatistics& outStatistics) const;

	virtual bool getBackBufferContent(void* buffer) const;

private:
	struct RenderTargetStack
	{
		RenderTargetSetOpenGLES2* renderTargetSet;
		int32_t renderTarget;
		Viewport viewport;
	};
	
	Ref< ContextOpenGLES2 > m_globalContext;
	Ref< ContextOpenGLES2 > m_context;
	Ref< StateCache > m_stateCache;
	std::stack< RenderTargetStack > m_renderTargetStack;
	Viewport m_viewport;
	bool m_landscape;
	bool m_cursorVisible;
	std::list< RenderEvent > m_eventQueue;

#if defined(_WIN32)
	bool windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult);
#endif
};

#endif

	}
}

#endif	// traktor_render_RenderViewOpenGLES2_H
