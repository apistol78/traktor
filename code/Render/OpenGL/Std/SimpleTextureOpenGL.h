#ifndef traktor_render_SimpleTextureOpenGL_H
#define traktor_render_SimpleTextureOpenGL_H

#include "Core/Math/Vector4.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/ISimpleTexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/ITextureBinding.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
class SimpleTextureOpenGL
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	SimpleTextureOpenGL(ContextOpenGL* resourceContext);

	virtual ~SimpleTextureOpenGL();
	
	bool create(const SimpleTextureCreateDesc& desc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void bindTexture(ContextOpenGL* renderContext, uint32_t samplerObject, uint32_t stage) T_OVERRIDE T_FINAL;

	virtual void bindSize(GLint locationSize) T_OVERRIDE T_FINAL;

private:
	Ref< ContextOpenGL > m_resourceContext;
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
