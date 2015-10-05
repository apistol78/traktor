#ifndef traktor_render_CubeTextureOpenGL_H
#define traktor_render_CubeTextureOpenGL_H

#include "Core/Misc/AutoPtr.h"
#include "Render/ICubeTexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/ITextureBinding.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
class CubeTextureOpenGL
:	public ICubeTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	CubeTextureOpenGL(ContextOpenGL* resourceContext);

	virtual ~CubeTextureOpenGL();

	bool create(const CubeTextureCreateDesc& desc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual int getDepth() const T_OVERRIDE T_FINAL;

	virtual bool lock(int side, int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int side, int level) T_OVERRIDE T_FINAL;
	
	// ITextureBinding
	
	virtual void bindTexture(ContextOpenGL* renderContext, uint32_t samplerObject, uint32_t stage) T_OVERRIDE T_FINAL;

	virtual void bindSize(GLint locationSize) T_OVERRIDE T_FINAL;
	
private:
	Ref< ContextOpenGL > m_resourceContext;
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
