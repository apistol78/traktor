#ifndef traktor_render_RenderTargetSetOpenGLES2_H
#define traktor_render_RenderTargetSetOpenGLES2_H

#include "Render/RenderTargetSet.h"
#include "Render/Types.h"
#include "Core/Heap/Ref.h"
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
	
#if !defined(T_OFFLINE_ONLY)

class IContext;
class RenderTargetOpenGLES2;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS RenderTargetSetOpenGLES2 : public RenderTargetSet
{
	T_RTTI_CLASS(RenderTargetSetOpenGLES2)

public:
	RenderTargetSetOpenGLES2(IContext* context);

	virtual ~RenderTargetSetOpenGLES2();

	bool create(const RenderTargetSetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual ITexture* getColorTexture(int index) const;

	virtual void swap(int index1, int index2);

	virtual bool read(int index, void* buffer) const;

private:
	Ref< IContext > m_context;
	int m_width;
	int m_height;
	GLuint m_depthBuffer;
	RefArray< RenderTargetOpenGLES2 > m_colorTextures;
};

#endif

	}
}

#endif	// traktor_render_RenderTargetSetOpenGLES2_H
