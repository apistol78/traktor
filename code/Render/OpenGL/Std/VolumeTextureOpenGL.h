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
#include "Render/OpenGL/Std/ITextureBinding.h"

namespace traktor
{
	namespace render
	{

class ResourceContextOpenGL;

/*!
 * \ingroup OGL
 */
class VolumeTextureOpenGL
:	public IVolumeTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;
	
public:
	VolumeTextureOpenGL(ResourceContextOpenGL* resourceContext);

	virtual ~VolumeTextureOpenGL();

	bool create(const VolumeTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;
	
	virtual int getDepth() const override final;

	// ITextureBinding

	virtual void bindTexture() const override final;

	virtual void bindSize(GLint locationSize) const override final;

	virtual bool haveMips() const override final;

private:
	Ref< ResourceContextOpenGL > m_resourceContext;
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
