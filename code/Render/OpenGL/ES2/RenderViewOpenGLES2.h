#pragma once

#include <list>
#include <stack>
#include "Render/IRenderView.h"
#include "Render/OpenGL/ES2/Platform.h"

#if defined(_WIN32)
#	include "Render/OpenGL/ES2/Win32/Window.h"
#elif defined(__LINUX__)
#	include "Render/OpenGL/ES2/Linux/Window.h"
#endif

namespace traktor
{
	namespace render
	{

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
		ContextOpenGLES2* context
	);

	virtual ~RenderViewOpenGLES2();

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
	struct RenderTargetStack
	{
		RenderTargetSetOpenGLES2* renderTargetSet;
		int32_t renderTarget;
		Viewport viewport;
	};

	Ref< ContextOpenGLES2 > m_context;
	Ref< StateCache > m_stateCache;
	std::stack< RenderTargetStack > m_renderTargetStack;
	Viewport m_viewport;
	int32_t m_width;
	int32_t m_height;
	bool m_cursorVisible;
	std::list< RenderEvent > m_eventQueue;
	uint32_t m_drawCalls;
	uint32_t m_primitiveCount;

#if defined(_WIN32)
	bool windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult);
#endif
};

	}
}
