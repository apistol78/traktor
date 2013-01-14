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

class IContext;

/*!
 * \ingroup OGL
 */
class RenderTargetOpenGLES2
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	RenderTargetOpenGLES2(IContext* context);

	virtual ~RenderTargetOpenGLES2();

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, GLuint depthBuffer, GLuint stencilBuffer);

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	virtual void bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture);

	virtual void bindSize(GLint locationSize);

	void bind(GLuint primaryDepthTarget);

	void enter();
	
	GLuint getColorTexture() { return m_colorTexture; }

private:
	Ref< IContext > m_context;
	int m_width;
	int m_height;
	bool m_usingPrimaryDepthBuffer;
	GLenum m_textureTarget;
	GLuint m_frameBufferObject;
	GLuint m_colorTexture;
	Vector4 m_originAndScale;
	bool m_haveDepth;
	SamplerStateOpenGL m_shadowState;
};

#endif

	}
}

#endif	// traktor_render_RenderTargetOpenGLES2_H
