#ifndef traktor_render_CubeTextureOpenGL_H
#define traktor_render_CubeTextureOpenGL_H

#include "Render/ICubeTexture.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IContext;
		
/*!
 * \ingroup OGL
 */
class T_DLLCLASS CubeTextureOpenGL : public ICubeTexture
{
	T_RTTI_CLASS;

public:
	CubeTextureOpenGL(IContext* context);

	virtual ~CubeTextureOpenGL();

	bool create(const CubeTextureCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual bool lock(int side, int level, Lock& lock);

	virtual void unlock(int side, int level);

	GLuint getTextureName() const { return m_textureName; }
	
private:
	Ref< IContext > m_context;
	GLuint m_textureName;
	int m_side;
	int m_pixelSize;
	GLint m_components;
	GLenum m_format;
	GLenum m_type;
	std::vector< unsigned char > m_data;
};

	}
}

#endif	// traktor_render_CubeTextureOpenGL_H
