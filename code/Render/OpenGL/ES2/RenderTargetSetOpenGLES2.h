#ifndef traktor_render_RenderTargetSetOpenGLES2_H
#define traktor_render_RenderTargetSetOpenGLES2_H

#include "Render/RenderTargetSet.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{
	
#if !defined(T_OFFLINE_ONLY)

class ContextOpenGLES2;
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

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual ISimpleTexture* getColorTexture(int index) const;

	virtual ISimpleTexture* getDepthTexture() const;

	virtual void swap(int index1, int index2);

	virtual void discard();

	virtual bool read(int index, void* buffer) const;

	bool bind(GLuint primaryDepthBuffer, int32_t renderTarget);

private:
	Ref< ContextOpenGLES2 > m_context;
	RenderTargetSetCreateDesc m_desc;
	GLuint m_targetFBO[8];
	GLuint m_depthBuffer;
	GLuint m_targetTextures[8];
	Ref< RenderTargetOpenGLES2 > m_renderTargets[8];

	bool createFramebuffer(GLuint primaryDepthBuffer);
};

#endif

	}
}

#endif	// traktor_render_RenderTargetSetOpenGLES2_H
