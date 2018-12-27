/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderTargetDepthOpenGLES2_H
#define traktor_render_RenderTargetDepthOpenGLES2_H

#include "Core/Math/Vector4.h"
#include "Render/ISimpleTexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/ES2/ITextureBinding.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGLES2;

/*!
 * \ingroup OGL
 */
class RenderTargetDepthOpenGLES2
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	RenderTargetDepthOpenGLES2(ContextOpenGLES2* resourceContext, GLuint depthTexture, int32_t width, int32_t height);

	virtual ~RenderTargetDepthOpenGLES2();

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;
	
	virtual bool lock(int level, Lock& lock) override final;

	virtual void unlock(int level) override final;

	virtual void* getInternalHandle() override final;

	virtual void bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture) override final;

	virtual void bindSize(GLint locationSize) override final;
	
private:
	Ref< ContextOpenGLES2 > m_resourceContext;
	GLuint m_depthTexture;
	int32_t m_width;
	int32_t m_height;
};

	}
}

#endif	// traktor_render_RenderTargetDepthOpenGLES2_H
