/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

class RenderContextOpenGL;
class RenderTargetDepthOpenGL;
class RenderTargetOpenGL;
class ResourceContextOpenGL;

/*!
 * \ingroup OGL
 */
class RenderTargetSetOpenGL : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	static uint32_t ms_primaryTargetTag;	//!< Tag to indicate state of primary depth buffer; each time the depth buffer is resized this needs to be incremented.

	RenderTargetSetOpenGL(ResourceContextOpenGL* resourceContext);

	virtual ~RenderTargetSetOpenGL();

	bool create(const RenderTargetSetCreateDesc& desc);

	virtual void destroy() override final;

	virtual int getWidth() const override final;

	virtual int getHeight() const override final;

	virtual ISimpleTexture* getColorTexture(int index) const override final;

	virtual ISimpleTexture* getDepthTexture() const override final;

	virtual void swap(int index1, int index2) override final;

	virtual void discard() override final;

	virtual bool isContentValid() const override final;

	virtual bool read(int index, void* buffer) const override final;

	bool bind(RenderContextOpenGL* renderContext, GLuint primaryDepthBuffer);

	bool bind(RenderContextOpenGL* renderContext, GLuint primaryDepthBuffer, int32_t renderTarget);

	void blit(RenderContextOpenGL* renderContext);

	GLuint getDepthBuffer() const { return m_depthBufferOrTexture; }

	void setContentValid(bool contentValid) { m_contentValid = contentValid; }

private:
	Ref< ResourceContextOpenGL > m_resourceContext;
	RenderTargetSetCreateDesc m_desc;
	GLuint m_targetFBO;
	GLuint m_depthBufferOrTexture;
	GLuint m_targetTextures[8];
	Ref< RenderTargetDepthOpenGL > m_depthTarget;
	Ref< RenderTargetOpenGL > m_colorTargets[8];
	uint32_t m_currentTag;
	bool m_contentValid;

	bool createFramebuffer(GLuint primaryDepthBuffer);
};

	}
}

#endif	// traktor_render_RenderTargetSetOpenGL_H
