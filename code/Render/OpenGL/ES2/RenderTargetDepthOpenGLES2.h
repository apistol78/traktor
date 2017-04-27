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
#include "Render/OpenGL/ITextureBinding.h"

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

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void* getInternalHandle() T_OVERRIDE T_FINAL;

	virtual void bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture) T_OVERRIDE T_FINAL;

	virtual void bindSize(GLint locationSize) T_OVERRIDE T_FINAL;
	
private:
	Ref< ContextOpenGLES2 > m_resourceContext;
	GLuint m_depthTexture;
	int32_t m_width;
	int32_t m_height;
};

	}
}

#endif	// traktor_render_RenderTargetDepthOpenGLES2_H
