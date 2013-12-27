#ifndef traktor_render_RenderViewOpenGL_H
#define traktor_render_RenderViewOpenGL_H

#include <list>
#include "Core/Containers/AlignedVector.h"
#include "Render/IRenderView.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"
#if defined(_WIN32)
#	include "Render/OpenGL/Std/Win32/Window.h"
#elif defined(__LINUX__)
#   include "Render/OpenGL/Std/Linux/Window.h"
#endif

namespace traktor
{
	namespace render
	{

class IndexBufferOpenGL;
class ProgramOpenGL;
class RenderSystemOpenGL;
class RenderTargetOpenGL;
class RenderTargetSetOpenGL;
class VertexBufferOpenGL;

/*!
 * \ingroup OGL
 */
class RenderViewOpenGL
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
		ContextOpenGL* resourceContext
	);

#elif defined(__APPLE__)

	RenderViewOpenGL(
		const RenderViewDesc desc,
		void* windowHandle,
		ContextOpenGL* renderContext,
		ContextOpenGL* resourceContext
	);

#elif defined(__LINUX__)

	RenderViewOpenGL(
		const RenderViewDesc desc,
		Window* window,
		ContextOpenGL* renderContext,
		ContextOpenGL* resourceContext
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
	struct TargetScope
	{
		Ref< RenderTargetSetOpenGL > renderTargetSet;
		int32_t renderTarget;
		uint32_t clearMask;
		Color4f clearColor;
		float clearDepth;
		int32_t clearStencil;
	};

#if defined(_WIN32)
	Ref< Window > m_window;
#elif defined(__APPLE__)
	void* m_windowHandle;
#elif defined(__LINUX__)
    Ref< Window > m_window;
#endif
	Ref< ContextOpenGL > m_renderContext;
	Ref< ContextOpenGL > m_resourceContext;
	RenderTargetSetCreateDesc m_primaryTargetDesc;
	Ref< RenderTargetSetOpenGL > m_primaryTarget;
	AlignedVector< TargetScope > m_targetStack;
	bool m_cursorVisible;
	bool m_waitVBlank;
	bool m_targetsDirty;
	uint32_t m_drawCalls;
	uint32_t m_primitiveCount;

	void bindTargets();

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
