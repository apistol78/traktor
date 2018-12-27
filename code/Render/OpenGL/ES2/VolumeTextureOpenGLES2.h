/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VolumeTextureOpenGLES2_H
#define traktor_render_VolumeTextureOpenGLES2_H

#include "Core/Misc/AutoPtr.h"
#include "Render/IVolumeTexture.h"
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
class VolumeTextureOpenGLES2
:	public IVolumeTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	VolumeTextureOpenGLES2(ContextOpenGLES2* resourceContext);

	virtual ~VolumeTextureOpenGLES2();

	bool create(const VolumeTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;
	
	virtual int getDepth() const override final;
	
	// ITextureBinding
	
	virtual void bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture) override final;

	virtual void bindSize(GLint locationSize) override final;
	
private:
	Ref< ContextOpenGLES2 > m_resourceContext;
	GLuint m_textureName;
	int32_t m_width;
	int32_t m_height;
	int32_t m_depth;
	int32_t m_pixelSize;
	uint32_t m_mipCount;
	GLint m_components;
	GLenum m_format;
	GLenum m_type;
	AutoArrayPtr< uint8_t > m_data;
	SamplerStateOpenGL m_shadowState;
};

	}
}

#endif	// traktor_render_VolumeTextureOpenGLES2_H
