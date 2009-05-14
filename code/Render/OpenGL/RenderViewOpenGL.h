#ifndef traktor_render_RenderViewOpenGL_H
#define traktor_render_RenderViewOpenGL_H

#include <stack>
#include "Core/Heap/Ref.h"
#include "Render/RenderView.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/ContextOpenGL.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class RenderSystemOpenGL;
class VertexBufferOpenGL;
class IndexBufferOpenGL;
class ProgramOpenGL;
class RenderTargetOpenGL;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS RenderViewOpenGL : public RenderView
{
	T_RTTI_CLASS(RenderViewOpenGL)

public:
#if defined(_WIN32)

	RenderViewOpenGL(
		ContextOpenGL* context,
		ContextOpenGL* globalContext,
		HWND hWnd
	);

#else

	RenderViewOpenGL(
		ContextOpenGL* context,
		ContextOpenGL* globalContext
	);

#endif

	virtual ~RenderViewOpenGL();

	virtual void close();

	virtual void resize(int32_t width, int32_t height);

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual bool begin();

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil);

	virtual void clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil);

	virtual void setVertexBuffer(VertexBuffer* vertexBuffer);

	virtual void setIndexBuffer(IndexBuffer* indexBuffer);

	virtual void setProgram(Program* program);

	virtual void draw(const Primitives& primitives);

	virtual void end();

	virtual void present();

private:
	Ref< RenderSystemOpenGL > m_renderSystem;
	Ref< ContextOpenGL > m_context;
	Ref< ContextOpenGL > m_globalContext;
	std::stack< RenderTargetOpenGL* > m_renderTargetStack;
	Ref< VertexBufferOpenGL > m_currentVertexBuffer;
	Ref< IndexBufferOpenGL > m_currentIndexBuffer;
	Ref< ProgramOpenGL > m_currentProgram;
	bool m_currentDirty;
};

	}
}

#endif	// traktor_render_RenderViewOpenGL_H
