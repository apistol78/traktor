#ifndef traktor_render_RenderViewOpenGL_H
#define traktor_render_RenderViewOpenGL_H

#include <list>
#include "Render/IRenderView.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"
#if defined(_WIN32)
#	include "Render/OpenGL/Std/Win32/Window.h"
#elif defined(__LINUX__)
#   include "Render/OpenGL/Std/Linux/Window.h"
#endif

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class BlitHelper;
class IndexBufferOpenGL;
class ProgramOpenGL;
class RenderSystemOpenGL;
class RenderTargetOpenGL;
class RenderTargetSetOpenGL;
class VertexBufferOpenGL;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS RenderViewOpenGL
:	public IRenderView
#if defined(_WIN32)
,	public IWindowListener
#endif
{
	T_RTTI_CLASS;

public:
#if defined(_WIN32)

	RenderViewOpenGL(
		const RenderViewDesc desc,
		Window* window,
		ContextOpenGL* renderContext,
		ContextOpenGL* resourceContext,
		BlitHelper* blitHelper
	);

#elif defined(__APPLE__)

	RenderViewOpenGL(
		const RenderViewDesc desc,
		void* windowHandle,
		ContextOpenGL* renderContext,
		ContextOpenGL* resourceContext,
		BlitHelper* blitHelper
	);

#elif defined(__LINUX__)

	RenderViewOpenGL(
		const RenderViewDesc desc,
		Window* window,
		ContextOpenGL* renderContext,
		ContextOpenGL* resourceContext,
		BlitHelper* blitHelper
	);

#endif

	bool createPrimaryTarget();

	virtual ~RenderViewOpenGL();

	virtual bool nextEvent(RenderEvent& outEvent);

	virtual void close();

	virtual bool reset(const RenderViewDefaultDesc& desc);

	virtual bool reset(int32_t width, int32_t height);

	virtual int getWidth() const;

	virtual int getHeight() const;

	virtual bool isActive() const;

	virtual bool isFullScreen() const;

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

private:
#if defined(_WIN32)
	Ref< Window > m_window;
#elif defined(__APPLE__)
	void* m_windowHandle;
#elif defined(__LINUX__)
    Ref< Window > m_window;
#endif
	Ref< ContextOpenGL > m_renderContext;
	Ref< ContextOpenGL > m_resourceContext;
	Ref< BlitHelper > m_blitHelper;
	RenderTargetSetCreateDesc m_primaryTargetDesc;
	Ref< RenderTargetSetOpenGL > m_primaryTarget;
	bool m_waitVBlank;
	std::vector< RenderTargetOpenGL* > m_renderTargetStack;

#if defined(_WIN32)

	std::list< RenderEvent > m_eventQueue;

	// \name IWindowListener implementation.
	// \{

	virtual bool windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult);

	// \}

#endif
};

	}
}

#endif	// traktor_render_RenderViewOpenGL_H
