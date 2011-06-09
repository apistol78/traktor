#ifndef traktor_render_RenderTargetSetOpenGLES2_H
#define traktor_render_RenderTargetSetOpenGLES2_H

#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"
#include "Render/RenderTargetSet.h"
#include "Render/Types.h"

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
	
#if !defined(T_OFFLINE_ONLY)

class IContext;
class RenderTargetOpenGLES2;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS RenderTargetSetOpenGLES2 : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetOpenGLES2(IContext* context);

	virtual ~RenderTargetSetOpenGLES2();

	bool create(const RenderTargetSetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual Ref< ITexture > getColorTexture(int index) const;

	virtual void swap(int index1, int index2);

	virtual bool read(int index, void* buffer) const;
	
	GLuint getDepthBuffer() const { return m_depthBuffer; }
	
	GLuint getClearMask() const { return m_clearMask; }

private:
	Ref< IContext > m_context;
	int m_width;
	int m_height;
	GLuint m_depthBuffer;
	GLuint m_stencilBuffer;
	RefArray< RenderTargetOpenGLES2 > m_colorTextures;
	GLuint m_clearMask;
};

#endif

	}
}

#endif	// traktor_render_RenderTargetSetOpenGLES2_H
