#pragma once

#include <list>
#include "Core/Containers/AlignedVector.h"
#include "Render/IRenderView.h"
#include "Render/OpenGL/Std/Platform.h"
#include "Render/OpenGL/Std/RenderContextOpenGL.h"
#include "Render/OpenGL/Std/ResourceContextOpenGL.h"
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
		RenderContextOpenGL* renderContext,
		ResourceContextOpenGL* resourceContext
	);
#elif defined(__APPLE__)
	RenderViewOpenGL(
		const RenderViewDesc& desc,
		void* windowHandle,
		RenderContextOpenGL* renderContext,
		ResourceContextOpenGL* resourceContext
	);
#elif defined(__LINUX__)
	RenderViewOpenGL(
		const RenderViewDesc& desc,
		Window* window,
		RenderContextOpenGL* renderContext,
		ResourceContextOpenGL* resourceContext
	);
#endif

	virtual ~RenderViewOpenGL();

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
#if defined(_WIN32)
	Ref< Window > m_window;
#elif defined(__APPLE__)
	void* m_windowHandle;
#elif defined(__LINUX__)
    Ref< Window > m_window;
#endif
	Ref< RenderContextOpenGL > m_renderContext;
	Ref< ResourceContextOpenGL > m_resourceContext;
	RenderTargetSetCreateDesc m_primaryTargetDesc;
	Ref< RenderTargetSetOpenGL > m_primaryTarget;
	Ref< RenderTargetSetOpenGL > m_activeTarget;
	bool m_cursorVisible;
	int32_t m_waitVBlanks;
	uint32_t m_passCount;
	uint32_t m_drawCalls;
	uint32_t m_primitiveCount;

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
