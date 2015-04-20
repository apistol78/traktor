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

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual bool lock(int side, int level, Lock& lock);

	virtual void unlock(int side, int level);
	
	// ITextureBinding
	
	virtual void bindTexture(ContextOpenGL* renderContext, uint32_t samplerObject, uint32_t stage);

	virtual void bindSize(GLint locationSize);
	
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
