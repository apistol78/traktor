/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_SimpleTextureOpenGLES2_H
#define traktor_render_SimpleTextureOpenGLES2_H

#include "Core/Containers/AlignedVector.h"
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
class SimpleTextureOpenGLES2
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	SimpleTextureOpenGLES2(ContextOpenGLES2* context);

	virtual ~SimpleTextureOpenGLES2();
	
	bool create(const SimpleTextureCreateDesc& desc);

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
	Ref< ContextOpenGLES2 > m_context;
	GLuint m_textureName;
	bool m_pot;
	int m_dirty;
	int m_width;
	int m_height;
	int m_pixelSize;
	GLint m_components;
	GLenum m_format;
	GLenum m_type;
	uint32_t m_mipCount;
	AlignedVector< uint8_t > m_data;
	SamplerStateOpenGL m_shadowState;
};
		
	}
}

#endif	// traktor_render_SimpleTextureOpenGLES2_H
