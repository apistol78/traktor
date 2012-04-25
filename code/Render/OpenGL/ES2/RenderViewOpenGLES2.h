#ifndef traktor_render_RenderViewOpenGLES2_H
#define traktor_render_RenderViewOpenGLES2_H

#include <stack>
#include "Render/IRenderView.h"
#include "Render/OpenGL/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_ES2_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
class T_DLLCLASS RenderViewOpenGLES2 : public IRenderView
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

	virtual bool setGamma(float gamma);

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual bool begin(EyeType eye);

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget);

	virtual void clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil);

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives);

	virtual void end();

	virtual void present();

	virtual void pushMarker(const char* const marker);

	virtual void popMarker();

	virtual void getStatistics(RenderViewStatistics& outStatistics) const;

private:
	struct RenderTargetStack
	{
		RenderTargetSetOpenGLES2* renderTargetSet;
		RenderTargetOpenGLES2* renderTarget;
		Viewport viewport;
	};
	
	Ref< ContextOpenGLES2 > m_globalContext;
	Ref< ContextOpenGLES2 > m_context;
	Ref< StateCache > m_stateCache;
	std::stack< RenderTargetStack > m_renderTargetStack;
	Viewport m_viewport;
};

#endif

	}
}

#endif	// traktor_render_RenderViewOpenGLES2_H
