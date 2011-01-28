#ifndef traktor_render_RenderTargetOpenGLES2_H
#define traktor_render_RenderTargetOpenGLES2_H

#include "Core/Math/Vector4.h"
#include "Render/ITexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/ITextureBinding.h"

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

class IContext;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS RenderTargetOpenGLES2
:	public ITexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	RenderTargetOpenGLES2(IContext* context);

	virtual ~RenderTargetOpenGLES2();

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, GLuint depthBuffer, GLuint stencilBuffer);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual void bind(GLuint unit, const SamplerState& samplerState, GLint locationTexture, GLint locationOffset);

	void bind();

	void enter();

private:
	Ref< IContext > m_context;
	int m_width;
	int m_height;
	GLenum m_textureTarget;
	GLuint m_frameBufferObject;
	GLuint m_colorTexture;
	Vector4 m_originAndScale;
	bool m_haveDepth;
	SamplerState m_shadowState;
};

#endif

	}
}

#endif	// traktor_render_RenderTargetOpenGLES2_H
