#ifndef traktor_render_SimpleTextureOpenGLES2_H
#define traktor_render_SimpleTextureOpenGLES2_H

#include "Core/Containers/AlignedVector.h"
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
class SimpleTextureOpenGLES2
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	SimpleTextureOpenGLES2(ContextOpenGLES2* context);

	virtual ~SimpleTextureOpenGLES2();
	
	bool create(const SimpleTextureCreateDesc& desc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture) T_OVERRIDE T_FINAL;

	virtual void bindSize(GLint locationSize) T_OVERRIDE T_FINAL;

private:
	Ref< ContextOpenGLES2 > m_context;
	GLuint m_textureName;
	bool m_pot;
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
