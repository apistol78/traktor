/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_SimpleTextureOpenGL_H
#define traktor_render_SimpleTextureOpenGL_H

#include "Core/Math/Vector4.h"
#include "Core/Misc/AutoPtr.h"
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
class SimpleTextureOpenGL
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	SimpleTextureOpenGL(ResourceContextOpenGL* resourceContext);

	virtual ~SimpleTextureOpenGL();
	
	bool create(const SimpleTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;

	virtual bool lock(int level, Lock& lock) override final;

	virtual void unlock(int level) override final;

	virtual void* getInternalHandle() override final;

	// ITextureBinding

	virtual void bindTexture() const override final;

	virtual void bindSize(GLint locationSize) const override final;

	virtual bool haveMips() const override final;

private:
	Ref< ResourceContextOpenGL > m_resourceContext;
	GLuint m_textureName;
	int m_width;
	int m_height;
	int m_pixelSize;
	GLint m_components;
	GLenum m_format;
	GLenum m_type;
	uint32_t m_mipCount;
	uint32_t m_dataSize;
	AutoArrayPtr< uint8_t > m_data;
};
		
	}
}

#endif	// traktor_render_SimpleTextureOpenGL_H
