#ifndef traktor_render_SimpleTextureOpenGLES2_H
#define traktor_render_SimpleTextureOpenGLES2_H

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

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	virtual void bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture);

	virtual void bindSize(GLint locationSize);

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
	std::vector< uint8_t > m_data;
	SamplerStateOpenGL m_shadowState;
};
		
	}
}

#endif	// traktor_render_SimpleTextureOpenGLES2_H
