#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Render/ISimpleTexture.h"
#include "Render/Types.h"
#include "Render/OpenGL/ES/ITextureBinding.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGLES;

/*!
 * \ingroup OGL
 */
class SimpleTextureOpenGLES
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	SimpleTextureOpenGLES(ContextOpenGLES* context);

	virtual ~SimpleTextureOpenGLES();

	bool create(const SimpleTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

	virtual void bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture) override final;

	virtual void bindSize(GLint locationSize) override final;

private:
	Ref< ContextOpenGLES > m_context;
	GLuint m_textureName;
	bool m_pot;
	int32_t m_dirty;
	int32_t m_width;
	int32_t m_height;
	int32_t m_pixelSize;
	GLint m_components;
	GLenum m_format;
	GLenum m_type;
	uint32_t m_mipCount;
	AlignedVector< uint8_t > m_data;
	SamplerStateOpenGL m_shadowState;
};

	}
}
