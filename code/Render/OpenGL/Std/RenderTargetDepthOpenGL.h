#ifndef traktor_render_RenderTargetDepthOpenGL_H
#define traktor_render_RenderTargetDepthOpenGL_H

#include "Core/Math/Vector4.h"
#include "Render/ISimpleTexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/ITextureBinding.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGL;

/*!
 * \ingroup OGL
 */
class RenderTargetDepthOpenGL
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	RenderTargetDepthOpenGL(ContextOpenGL* resourceContext, GLuint depthTexture, int32_t width, int32_t height);

	virtual ~RenderTargetDepthOpenGL();

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	virtual void bindTexture(ContextOpenGL* renderContext, uint32_t samplerObject);

	virtual void bindSize(GLint locationSize);
	
private:
	Ref< ContextOpenGL > m_resourceContext;
	GLuint m_depthTexture;
	int32_t m_width;
	int32_t m_height;
};

	}
}

#endif	// traktor_render_RenderTargetDepthOpenGL_H
