#ifndef traktor_render_VolumeTextureOpenGLES2_H
#define traktor_render_VolumeTextureOpenGLES2_H

#include "Core/Misc/AutoPtr.h"
#include "Render/IVolumeTexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/ITextureBinding.h"

namespace traktor
{
	namespace render
	{

class IContext;
		
/*!
 * \ingroup OGL
 */
class VolumeTextureOpenGLES2
:	public IVolumeTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	VolumeTextureOpenGLES2(IContext* resourceContext);

	virtual ~VolumeTextureOpenGLES2();

	bool create(const VolumeTextureCreateDesc& desc);

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual bool lock(int side, int level, Lock& lock);

	virtual void unlock(int side, int level);
	
	// ITextureBinding
	
	virtual void bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture);

	virtual void bindSize(GLint locationSize);
	
private:
	Ref< IContext > m_resourceContext;
	GLuint m_textureName;
	int32_t m_width;
	int32_t m_height;
	int32_t m_depth;
	int32_t m_pixelSize;
	uint32_t m_mipCount;
	GLint m_components;
	GLenum m_format;
	GLenum m_type;
	AutoArrayPtr< uint8_t > m_data;
	SamplerStateOpenGL m_shadowState;
};

	}
}

#endif	// traktor_render_VolumeTextureOpenGLES2_H
