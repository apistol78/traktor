#pragma once

#include "Render/RenderTargetSet.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGLES2;
class RenderTargetDepthOpenGLES2;
class RenderTargetOpenGLES2;

/*!
 * \ingroup OGL
 */
class RenderTargetSetOpenGLES2 : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetOpenGLES2(ContextOpenGLES2* context);

	virtual ~RenderTargetSetOpenGLES2();

	bool create(const RenderTargetSetCreateDesc& desc);

	virtual void destroy() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual ISimpleTexture* getColorTexture(int32_t index) const override final;

	virtual ISimpleTexture* getDepthTexture() const override final;

	virtual void discard() override final;

	virtual bool isContentValid() const override final;

	virtual bool read(int32_t index, void* buffer) const override final;

	bool bind(GLuint primaryDepthBuffer, int32_t renderTarget);

	void setContentValid(bool contentValid) { m_contentValid = contentValid; }

private:
	Ref< ContextOpenGLES2 > m_context;
	RenderTargetSetCreateDesc m_desc;
	GLuint m_targetFBO[8];
	GLuint m_depthBufferOrTexture;
	GLuint m_targetTextures[8];
	Ref< RenderTargetDepthOpenGLES2 > m_depthTarget;
	Ref< RenderTargetOpenGLES2 > m_renderTargets[8];
	bool m_contentValid;

	bool createFramebuffer(GLuint primaryDepthBuffer);
};

	}
}

