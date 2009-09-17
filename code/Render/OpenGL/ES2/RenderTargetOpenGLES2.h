#ifndef traktor_render_RenderTargetOpenGLES2_H
#define traktor_render_RenderTargetOpenGLES2_H

#include "Core/Heap/Ref.h"
#include "Render/ITexture.h"
#include "Render/Types.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_ES2_EXPORT)
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
class T_DLLCLASS RenderTargetOpenGLES2 : public ITexture
{
	T_RTTI_CLASS(RenderTargetOpenGLES2)

public:
	RenderTargetOpenGLES2(IContext* context);

	virtual ~RenderTargetOpenGLES2();

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, GLuint depthBuffer);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	void bind();

	void enter();

	inline GLenum getTextureTarget() const { return m_textureTarget; }

	inline GLuint getTextureName() const { return m_colorTexture; }

	inline const Vector4& getTextureOriginAndScale() const { return m_originAndScale; }

private:
	Ref< IContext > m_context;
	int m_width;
	int m_height;
	GLenum m_textureTarget;
	GLuint m_frameBufferObject;
	GLuint m_colorTexture;
	Vector4 m_originAndScale;
	bool m_haveDepth;
};

	}
}

#endif	// traktor_render_RenderTargetOpenGLES2_H
