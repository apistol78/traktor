#pragma once

#include "Core/Math/Vector4.h"
#include "Render/ISimpleTexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/ES/ITextureBinding.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGLES;

/*!
 * \ingroup OGL
 */
class RenderTargetOpenGLES
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	RenderTargetOpenGLES(ContextOpenGLES* context, GLuint colorTexture, int32_t width, int32_t height);

	virtual ~RenderTargetOpenGLES();

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

	virtual void bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture) override final;

	virtual void bindSize(GLint locationSize) override final;

private:
	Ref< ContextOpenGLES > m_context;
	GLuint m_colorTexture;
	int32_t m_width;
	int32_t m_height;
	SamplerStateOpenGL m_shadowState;
};

	}
}
