#ifndef traktor_render_RenderTargetSetOpenGL_H
#define traktor_render_RenderTargetSetOpenGL_H

#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"
#include "Render/RenderTargetSet.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGL;
class RenderTargetDepthOpenGL;
class RenderTargetOpenGL;

/*!
 * \ingroup OGL
 */
class RenderTargetSetOpenGL : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	static uint32_t ms_primaryTargetTag;	//!< Tag to indicate state of primary depth buffer; each time the depth buffer is resized this needs to be incremented.

	RenderTargetSetOpenGL(ContextOpenGL* resourceContext);

	virtual ~RenderTargetSetOpenGL();

	bool create(const RenderTargetSetCreateDesc& desc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;

	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual ISimpleTexture* getColorTexture(int index) const T_OVERRIDE T_FINAL;

	virtual ISimpleTexture* getDepthTexture() const T_OVERRIDE T_FINAL;

	virtual void swap(int index1, int index2) T_OVERRIDE T_FINAL;

	virtual void discard() T_OVERRIDE T_FINAL;

	virtual bool read(int index, void* buffer) const T_OVERRIDE T_FINAL;

	bool bind(ContextOpenGL* renderContext, GLuint primaryDepthBuffer);

	bool bind(ContextOpenGL* renderContext, GLuint primaryDepthBuffer, int32_t renderTarget);

	void blit(ContextOpenGL* renderContext);

	GLuint getDepthBuffer() const { return m_depthBufferOrTexture; }

private:
	Ref< ContextOpenGL > m_resourceContext;
	RenderTargetSetCreateDesc m_desc;
	GLuint m_targetFBO;
	GLuint m_depthBufferOrTexture;
	GLuint m_targetTextures[8];
	Ref< RenderTargetDepthOpenGL > m_depthTarget;
	Ref< RenderTargetOpenGL > m_colorTargets[8];
	uint32_t m_currentTag;

	bool createFramebuffer(GLuint primaryDepthBuffer);
};

	}
}

#endif	// traktor_render_RenderTargetSetOpenGL_H
