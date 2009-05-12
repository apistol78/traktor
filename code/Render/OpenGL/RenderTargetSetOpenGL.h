#ifndef traktor_render_RenderTargetSetOpenGL_H
#define traktor_render_RenderTargetSetOpenGL_H

#include "Render/RenderTargetSet.h"
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
class RenderTargetOpenGL;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS RenderTargetSetOpenGL : public RenderTargetSet
{
	T_RTTI_CLASS(RenderTargetSetOpenGL)

public:
	RenderTargetSetOpenGL(ContextOpenGL* context);

	virtual ~RenderTargetSetOpenGL();

	bool create(const RenderTargetSetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual Texture* getColorTexture(int index) const;

private:
	Ref< ContextOpenGL > m_context;
	int m_width;
	int m_height;
	GLuint m_depthBuffer;
	RefArray< RenderTargetOpenGL > m_colorTextures;
};

	}
}

#endif	// traktor_render_RenderTargetSetOpenGL_H
