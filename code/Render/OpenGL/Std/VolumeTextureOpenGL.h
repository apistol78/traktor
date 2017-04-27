/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VolumeTextureOpenGL_H
#define traktor_render_VolumeTextureOpenGL_H

#include "Core/Misc/AutoPtr.h"
#include "Render/IVolumeTexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/ITextureBinding.h"

namespace traktor
{
	namespace render
	{
		
/*!
 * \ingroup OGL
 */
class VolumeTextureOpenGL
:	public IVolumeTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;
	
public:
	VolumeTextureOpenGL(ContextOpenGL* resourceContext);

	virtual ~VolumeTextureOpenGL();

	bool create(const VolumeTextureCreateDesc& desc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual int getDepth() const T_OVERRIDE T_FINAL;

	virtual void bindTexture(ContextOpenGL* renderContext, uint32_t samplerObject, uint32_t stage) T_OVERRIDE T_FINAL;

	virtual void bindSize(GLint locationSize) T_OVERRIDE T_FINAL;

private:
	Ref< ContextOpenGL > m_resourceContext;
	GLuint m_textureName;
	int m_width;
	int m_height;
	int m_depth;
	int32_t m_pixelSize;
	GLint m_components;
	GLenum m_format;
	GLenum m_type;
};
		
	}
}

#endif	// traktor_render_VolumeTextureOpenGL_H
