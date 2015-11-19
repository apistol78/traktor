#ifndef traktor_render_RenderTargetOpenGLES2_H
#define traktor_render_RenderTargetOpenGLES2_H

#include "Core/Math/Vector4.h"
#include "Render/ISimpleTexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/ITextureBinding.h"

namespace traktor
{
	namespace render
	{

#if !defined(T_OFFLINE_ONLY)

class ContextOpenGLES2;

/*!
 * \ingroup OGL
 */
class RenderTargetOpenGLES2
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	RenderTargetOpenGLES2(ContextOpenGLES2* context, GLuint colorTexture, int32_t width, int32_t height);

	virtual ~RenderTargetOpenGLES2();

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	virtual void bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture);

	virtual void bindSize(GLint locationSize);

private:
	Ref< ContextOpenGLES2 > m_context;
	GLuint m_colorTexture;
	int32_t m_width;
	int32_t m_height;
	SamplerStateOpenGL m_shadowState;
};

#endif

	}
}

#endif	// traktor_render_RenderTargetOpenGLES2_H
