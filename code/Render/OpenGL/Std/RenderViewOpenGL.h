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
		const RenderViewDesc& desc,
		Window* window,
		ContextOpenGL* renderContext,
		ContextOpenGL* resourceContext
	);

#elif defined(__APPLE__)

	RenderViewOpenGL(
		const RenderViewDesc& desc,
		void* windowHandle,
		ContextOpenGL* renderContext,
		ContextOpenGL* resourceContext
	);

#elif defined(__LINUX__)

	RenderViewOpenGL(
		const RenderViewDesc& desc,
		Window* window,
		ContextOpenGL* renderContext,
		ContextOpenGL* resourceContext
	);

#endif

	virtual ~RenderViewOpenGL();

	virtual bool nextEvent(RenderEvent& outEvent) T_OVERRIDE T_FINAL;

	virtual void close() T_OVERRIDE T_FINAL;

	virtual bool reset(const RenderViewDefaultDesc& desc) T_OVERRIDE T_FINAL;

	virtual bool reset(int32_t width, int32_t height) T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;

	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual bool isActive() const T_OVERRIDE T_FINAL;

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
	int32_t m_waitVBlanks;
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
