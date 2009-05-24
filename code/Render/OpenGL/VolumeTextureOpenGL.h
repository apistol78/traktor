#ifndef traktor_render_VolumeTextureOpenGL_H
#define traktor_render_VolumeTextureOpenGL_H

#include "Core/Heap/Ref.h"
#include "Render/VolumeTexture.h"
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
		
class ContextOpenGL;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS VolumeTextureOpenGL : public VolumeTexture
{
	T_RTTI_CLASS(VolumeTextureOpenGL)
	
public:
	VolumeTextureOpenGL(ContextOpenGL* context);

	bool create(const VolumeTextureCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	GLuint getTextureName() const { return m_textureName; }
	
private:
	Ref< ContextOpenGL > m_context;
	GLuint m_textureName;
	int m_width;
	int m_height;
	int m_depth;
};
		
	}
}

#endif	// traktor_render_VolumeTextureOpenGL_H
