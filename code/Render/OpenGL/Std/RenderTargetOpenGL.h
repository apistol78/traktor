#ifndef traktor_render_RenderTargetOpenGL_H
#define traktor_render_RenderTargetOpenGL_H

#include "Core/Math/Vector4.h"
#include "Render/ISimpleTexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/ITextureBinding.h"

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

class BlitHelper;
class IContext;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS RenderTargetOpenGL
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	RenderTargetOpenGL(IContext* resourceContext, BlitHelper* blitHelper);

	virtual ~RenderTargetOpenGL();

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, GLuint depthBuffer, bool backBuffer);

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	virtual void bind(GLuint unit, const SamplerState& samplerState, GLint locationTexture);

	bool bind(GLuint depthBuffer);

	void enter(GLuint depthBuffer);
	
	void resolveTarget();

	void blit();
	
	bool read(void* buffer) const;
	
	GLuint clearMask() const;

private:
	Ref< IContext > m_resourceContext;
	Ref< BlitHelper > m_blitHelper;
	int32_t m_width;
	int32_t m_height;
	int32_t m_targetWidth;
	int32_t m_targetHeight;
	GLenum m_textureTarget;
	GLuint m_targetFBO;
	GLuint m_resolveFBO;
	GLuint m_targetColorBuffer;
	GLuint m_colorTexture;
	bool m_haveDepth;
	bool m_usingPrimaryDepthBuffer;
	bool m_haveBlitExt;
	SamplerState m_shadowState;
};

	}
}

#endif	// traktor_render_RenderTargetOpenGL_H
