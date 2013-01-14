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

class BlitHelper;
class ContextOpenGL;
class RenderTargetOpenGL;

/*!
 * \ingroup OGL
 */
class RenderTargetSetOpenGL : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetOpenGL(ContextOpenGL* resourceContext, BlitHelper* blitHelper);

	virtual ~RenderTargetSetOpenGL();

	bool create(const RenderTargetSetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual ISimpleTexture* getColorTexture(int index) const;

	virtual void swap(int index1, int index2);

	virtual bool read(int index, void* buffer) const;

	bool bind(ContextOpenGL* renderContext, GLuint primaryDepthBuffer);

	bool bind(ContextOpenGL* renderContext, GLuint primaryDepthBuffer, int32_t renderTarget);

	void resolve();

	void blit();

	GLuint getDepthBuffer() const { return m_depthBuffer; }

private:
	Ref< ContextOpenGL > m_resourceContext;
	Ref< BlitHelper > m_blitHelper;
	bool m_haveBlitExt;
	int32_t m_width;
	int32_t m_height;
	GLuint m_targetFBO;
	GLuint m_depthBuffer;
	bool m_usingPrimaryDepth;
	GLuint m_targetCount;
	GLuint m_targetTextures[8];
	Ref< RenderTargetOpenGL > m_renderTargets[8];
};

	}
}

#endif	// traktor_render_RenderTargetSetOpenGL_H
