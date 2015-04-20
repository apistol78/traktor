#ifndef traktor_render_VolumeTextureOpenGL_H
#define traktor_render_VolumeTextureOpenGL_H

#include "Core/Misc/AutoPtr.h"
#include "Render/IVolumeTexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/ITextureBinding.h"

namespace traktor
{
	namespace render
	{
		
/*!
 * \ingroup OGL
 */
class VolumeTextureOpenGL
:	public IVolumeTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;
	
public:
	VolumeTextureOpenGL(ContextOpenGL* resourceContext);

	virtual ~VolumeTextureOpenGL();

	bool create(const VolumeTextureCreateDesc& desc);

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual void bindTexture(ContextOpenGL* renderContext, uint32_t samplerObject, uint32_t stage);

	virtual void bindSize(GLint locationSize);

private:
	Ref< ContextOpenGL > m_resourceContext;
	GLuint m_textureName;
	int m_width;
	int m_height;
	int m_depth;
	int32_t m_pixelSize;
	GLint m_components;
	GLenum m_format;
	GLenum m_type;
};
		
	}
}

#endif	// traktor_render_VolumeTextureOpenGL_H
