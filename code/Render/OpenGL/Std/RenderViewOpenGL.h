#ifndef traktor_render_RenderViewOpenGL_H
#define traktor_render_RenderViewOpenGL_H

#include <stack>
#include "Render/IRenderView.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"

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

class RenderSystemOpenGL;
class VertexBufferOpenGL;
class IndexBufferOpenGL;
class ProgramOpenGL;
class RenderTargetOpenGL;
class RenderTargetSetOpenGL;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS RenderViewOpenGL : public IRenderView
{
	T_RTTI_CLASS;

public:
#if defined(_WIN32)

	RenderViewOpenGL(
		const RenderViewDesc desc,
		ContextOpenGL* context,
		ContextOpenGL* resourceContext,
		HWND hWnd
	);

#elif defined(__APPLE__)

	RenderViewOpenGL(
		const RenderViewDesc desc,
		ContextOpenGL* context,
		ContextOpenGL* resourceContext,
		void* windowHandle
	);

#else

	RenderViewOpenGL(
		const RenderViewDesc desc,
		ContextOpenGL* context,
		ContextOpenGL* resourceContext
	);

#endif

	bool createPrimaryTarget();

	virtual ~RenderViewOpenGL();

	virtual void close();

	virtual bool reset(const RenderViewDefaultDesc& desc);

	virtual void resize(int32_t width, int32_t height);

	virtual int getWidth() const;

	virtual int getHeight() const;

	virtual bool isActive() const;

	virtual bool isFullScreen() const;

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual bool begin(EyeType eye);

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil);

	virtual void clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil);

	virtual void setVertexBuffer(VertexBuffer* vertexBuffer);

	virtual void setIndexBuffer(IndexBuffer* indexBuffer);

	virtual void setProgram(IProgram* program);

	virtual void draw(const Primitives& primitives);

	virtual void end();

	virtual void present();

	virtual void pushMarker(const char* const marker);

	virtual void popMarker();

	virtual void getStatistics(RenderViewStatistics& outStatistics) const;

private:
	Ref< RenderSystemOpenGL > m_renderSystem;
	Ref< ContextOpenGL > m_context;
	Ref< ContextOpenGL > m_resourceContext;
	
#if defined(__APPLE__)
	void* m_windowHandle;
#endif
	
	RenderTargetSetCreateDesc m_primaryTargetDesc;
	Ref< RenderTargetSetOpenGL > m_primaryTarget;
	
	bool m_waitVBlank;

	std::stack< RenderTargetOpenGL* > m_renderTargetStack;
	Ref< VertexBufferOpenGL > m_currentVertexBuffer;
	Ref< IndexBufferOpenGL > m_currentIndexBuffer;
	Ref< ProgramOpenGL > m_currentProgram;
	bool m_currentDirty;
};

	}
}

#endif	// traktor_render_RenderViewOpenGL_H
