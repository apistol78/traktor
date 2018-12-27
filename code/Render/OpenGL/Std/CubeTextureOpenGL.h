/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_CubeTextureOpenGL_H
#define traktor_render_CubeTextureOpenGL_H

#include "Core/Misc/AutoPtr.h"
#include "Render/ICubeTexture.h"
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
class CubeTextureOpenGL
:	public ICubeTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	CubeTextureOpenGL(ResourceContextOpenGL* resourceContext);

	virtual ~CubeTextureOpenGL();

	bool create(const CubeTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;
	
	virtual int getDepth() const override final;

	virtual bool lock(int side, int level, Lock& lock) override final;

	virtual void unlock(int side, int level) override final;
	
	// ITextureBinding
	
	virtual void bindTexture() const override final;

	virtual void bindSize(GLint locationSize) const override final;

	virtual bool haveMips() const override final;
	
private:
	Ref< ResourceContextOpenGL > m_resourceContext;
	GLuint m_textureName;
	int32_t m_side;
	int32_t m_pixelSize;
	uint32_t m_mipCount;
	GLint m_components;
	GLenum m_format;
	GLenum m_type;
	AutoArrayPtr< uint8_t > m_data;
};

	}
}

#endif	// traktor_render_CubeTextureOpenGL_H
