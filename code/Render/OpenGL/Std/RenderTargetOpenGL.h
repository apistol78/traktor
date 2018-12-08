/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderTargetOpenGL_H
#define traktor_render_RenderTargetOpenGL_H

#include "Core/Math/Vector4.h"
#include "Render/ISimpleTexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/Std/ITextureBinding.h"

namespace traktor
{
	namespace render
	{

class ResourceContextOpenGL;

/*!
 * \ingroup OGL
 */
class RenderTargetOpenGL
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	RenderTargetOpenGL(ResourceContextOpenGL* resourceContext, GLuint colorTexture, int32_t width, int32_t height);

	virtual ~RenderTargetOpenGL();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void* getInternalHandle() T_OVERRIDE T_FINAL;

	// ITextureBinding

	virtual void bindTexture() const T_OVERRIDE T_FINAL;

	virtual void bindSize(GLint locationSize) const T_OVERRIDE T_FINAL;

	virtual bool haveMips() const T_OVERRIDE T_FINAL;
	
private:
	Ref< ResourceContextOpenGL > m_resourceContext;
	GLuint m_colorTexture;
	int32_t m_width;
	int32_t m_height;
};

	}
}

#endif	// traktor_render_RenderTargetOpenGL_H
