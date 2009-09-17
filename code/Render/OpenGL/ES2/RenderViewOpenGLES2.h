#ifndef traktor_render_RenderViewOpenGLES2_H
#define traktor_render_RenderViewOpenGLES2_H

#include <stack>
#include "Core/Heap/Ref.h"
#include "Render/IRenderView.h"
#include "Render/OpenGL/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_ES2_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class VertexBufferOpenGLES2;
class IndexBufferOpenGLES2;
class ProgramOpenGLES2;
class RenderTargetOpenGLES2;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS RenderViewOpenGLES2 : public IRenderView
{
	T_RTTI_CLASS(RenderViewOpenGLES2)

public:
	RenderViewOpenGLES2(EGLDisplay display, EGLContext context, EGLSurface surface);

	virtual ~RenderViewOpenGLES2();

	virtual void close();

	virtual void resize(int32_t width, int32_t height);

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual bool begin();

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil);

	virtual void clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil);

	virtual void setVertexBuffer(VertexBuffer* vertexBuffer);

	virtual void setIndexBuffer(IndexBuffer* indexBuffer);

	virtual void setProgram(IProgram* program);

	virtual void draw(const Primitives& primitives);

	virtual void end();

	virtual void present();

private:
	EGLDisplay m_display;
	EGLContext m_context;
	EGLSurface m_surface;
	std::stack< RenderTargetOpenGLES2* > m_renderTargetStack;
	Ref< VertexBufferOpenGLES2 > m_currentVertexBuffer;
	Ref< IndexBufferOpenGLES2 > m_currentIndexBuffer;
	Ref< ProgramOpenGLES2 > m_currentProgram;
	bool m_currentDirty;
};

	}
}

#endif	// traktor_render_RenderViewOpenGLES2_H
