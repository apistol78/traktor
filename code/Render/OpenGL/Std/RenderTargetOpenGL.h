#pragma once

#include "Core/Math/Vector4.h"
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
class RenderTargetOpenGL
:	public ISimpleTexture
,	public ITextureBinding
{
	T_RTTI_CLASS;

public:
	RenderTargetOpenGL(ResourceContextOpenGL* resourceContext, GLuint colorTexture, int32_t width, int32_t height);

	virtual ~RenderTargetOpenGL();

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;
	
	virtual bool lock(int level, Lock& lock) override final;

	virtual void unlock(int level) override final;

	virtual void* getInternalHandle() override final;

	// ITextureBinding

	virtual void bindTexture(GLuint textureUnit) const override final;

	virtual void bindImage(GLuint imageUnit) const override final;

	virtual void bindSize(GLint locationSize) const override final;

	virtual bool haveMips() const override final;
	
private:
	Ref< ResourceContextOpenGL > m_resourceContext;
	GLuint m_colorTexture;
	int32_t m_width;
	int32_t m_height;
};

	}
}
