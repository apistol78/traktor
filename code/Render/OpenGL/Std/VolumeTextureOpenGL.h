#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Render/IVolumeTexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/Std/ITextureBinding.h"

namespace traktor
{
	namespace render
	{

class ResourceContextOpenGL;

/*!
 * \ingroup OGL
 */
class VolumeTextureOpenGL
:	public IVolumeTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	VolumeTextureOpenGL(ResourceContextOpenGL* resourceContext);

	virtual ~VolumeTextureOpenGL();

	bool create(const VolumeTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual int32_t getDepth() const override final;

	// ITextureBinding

	virtual void bindTexture(GLuint textureUnit) const override final;

	virtual void bindImage(GLuint imageUnit) const override final;

	virtual void bindSize(GLint locationSize) const override final;

	virtual bool haveMips() const override final;

private:
	Ref< ResourceContextOpenGL > m_resourceContext;
	GLuint m_textureName;
	int32_t m_width;
	int32_t m_height;
	int32_t m_depth;
	int32_t m_pixelSize;
	GLint m_components;
	GLenum m_format;
	GLenum m_type;
};

	}
}
