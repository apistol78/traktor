#pragma once

#include "Render/IRenderTargetSet.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGLES;
class RenderTargetDepthOpenGLES;
class RenderTargetOpenGLES;

/*!
 * \ingroup OGL
 */
class RenderTargetSetOpenGLES : public IRenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetOpenGLES(ContextOpenGLES* context);

	virtual ~RenderTargetSetOpenGLES();

	bool create(const RenderTargetSetCreateDesc& desc, IRenderTargetSet* sharedDepthStencil);

	virtual void destroy() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual ISimpleTexture* getColorTexture(int32_t index) const override final;

	virtual ISimpleTexture* getDepthTexture() const override final;

	virtual bool isContentValid() const override final;

	virtual bool read(int32_t index, void* buffer) const override final;

	bool bind(GLuint primaryDepthBuffer, int32_t renderTarget);

	void setContentValid(bool contentValid) { m_contentValid = contentValid; }

private:
	Ref< ContextOpenGLES > m_context;
	RenderTargetSetCreateDesc m_desc;
	GLuint m_targetFBO[8];
	GLuint m_depthBufferOrTexture;
	GLuint m_targetTextures[8];
	Ref< RenderTargetDepthOpenGLES > m_depthTarget;
	Ref< RenderTargetOpenGLES > m_renderTargets[8];
	bool m_contentValid;

	bool createFramebuffer(GLuint primaryDepthBuffer);
};

	}
}

