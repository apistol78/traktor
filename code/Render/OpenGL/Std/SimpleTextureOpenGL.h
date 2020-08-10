#pragma once

#include "Core/Math/Vector4.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/ISimpleTexture.h"
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
class SimpleTextureOpenGL
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	SimpleTextureOpenGL(ResourceContextOpenGL* resourceContext);

	virtual ~SimpleTextureOpenGL();

	bool create(const SimpleTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

	// ITextureBinding

	virtual void bindTexture(GLuint textureUnit) const override final;

	virtual void bindImage(GLuint imageUnit) const override final;

	virtual void bindSize(GLint locationSize) const override final;

	virtual bool haveMips() const override final;

	GLuint getTextureName() const { return m_textureName; }

private:
	Ref< ResourceContextOpenGL > m_resourceContext;
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
