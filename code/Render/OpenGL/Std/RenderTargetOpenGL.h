#ifndef traktor_render_RenderTargetOpenGL_H
#define traktor_render_RenderTargetOpenGL_H

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
class RenderTargetOpenGL
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	RenderTargetOpenGL(ContextOpenGL* resourceContext, GLuint colorTexture, int32_t width, int32_t height);

	virtual ~RenderTargetOpenGL();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void bindTexture(ContextOpenGL* renderContext, uint32_t samplerObject, uint32_t stage) T_OVERRIDE T_FINAL;

	virtual void bindSize(GLint locationSize) T_OVERRIDE T_FINAL;
	
private:
	Ref< ContextOpenGL > m_resourceContext;
	GLuint m_colorTexture;
	int32_t m_width;
	int32_t m_height;
};

	}
}

#endif	// traktor_render_RenderTargetOpenGL_H
