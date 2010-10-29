#ifndef traktor_render_RenderTargetSetOpenGL_H
#define traktor_render_RenderTargetSetOpenGL_H

#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"
#include "Render/RenderTargetSet.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IContext;
class RenderTargetOpenGL;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS RenderTargetSetOpenGL : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetOpenGL(IContext* resourceContext);

	virtual ~RenderTargetSetOpenGL();

	bool create(const RenderTargetSetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual Ref< ITexture > getColorTexture(int index) const;

	virtual void swap(int index1, int index2);

	virtual bool read(int index, void* buffer) const;

	GLuint getDepthBuffer() const { return m_depthBuffer; }

private:
	Ref< IContext > m_resourceContext;
	int32_t m_width;
	int32_t m_height;
	int32_t m_targetWidth;
	int32_t m_targetHeight;
	GLuint m_depthBuffer;
	RefArray< RenderTargetOpenGL > m_colorTextures;
};

	}
}

#endif	// traktor_render_RenderTargetSetOpenGL_H
