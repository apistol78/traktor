#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Render/ICubeTexture.h"
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
class CubeTextureOpenGL
:	public ICubeTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	CubeTextureOpenGL(ResourceContextOpenGL* resourceContext);

	virtual ~CubeTextureOpenGL();

	bool create(const CubeTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getSide() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	// ITextureBinding

	virtual void bindTexture(GLuint textureUnit) const override final;

	virtual void bindImage(GLuint imageUnit) const override final;

	virtual void bindSize(GLint locationSize) const override final;

	virtual bool haveMips() const override final;

	GLuint getTextureName() const { return m_textureName; }

private:
	Ref< ResourceContextOpenGL > m_resourceContext;
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
