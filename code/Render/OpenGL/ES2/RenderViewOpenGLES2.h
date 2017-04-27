/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	virtual void clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil) T_OVERRIDE T_FINAL;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives) T_OVERRIDE T_FINAL;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount) T_OVERRIDE T_FINAL;

	virtual void end() T_OVERRIDE T_FINAL;

	virtual void present() T_OVERRIDE T_FINAL;

	virtual void pushMarker(const char* const marker) T_OVERRIDE T_FINAL;

	virtual void popMarker() T_OVERRIDE T_FINAL;

	virtual void getStatistics(RenderViewStatistics& outStatistics) const T_OVERRIDE T_FINAL;

	virtual bool getBackBufferContent(void* buffer) const T_OVERRIDE T_FINAL;

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

#endif	// traktor_render_RenderViewOpenGLES2_H
