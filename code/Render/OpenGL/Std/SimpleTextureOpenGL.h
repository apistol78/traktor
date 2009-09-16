#ifndef traktor_render_SimpleTextureOpenGL_H
#define traktor_render_SimpleTextureOpenGL_H

#include "Render/ISimpleTexture.h"
#include "Render/Types.h"
#include "Core/Heap/Ref.h"
#include "Core/Math/Vector4.h"

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
class T_DLLCLASS SimpleTextureOpenGL : public ISimpleTexture
{
	T_RTTI_CLASS(ISimpleTexture)

public:
	SimpleTextureOpenGL(ContextOpenGL* context);

	virtual ~SimpleTextureOpenGL();
	
	bool create(const SimpleTextureCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	GLuint getTextureName() const { return m_textureName; }

	Vector4 getTextureOriginAndScale() const { return Vector4(0.0f, 0.0f, 1.0f, 1.0f);	}

	uint32_t getMipCount() const { return m_mipCount; }

private:
	Ref< ContextOpenGL > m_context;
	GLuint m_textureName;
	int m_width;
	int m_height;
	int m_pixelSize;
	GLint m_components;
	GLenum m_format;
	GLenum m_type;
	uint32_t m_mipCount;
	std::vector< uint8_t > m_data;
};
		
	}
}

#endif	// traktor_render_SimpleTextureOpenGL_H
