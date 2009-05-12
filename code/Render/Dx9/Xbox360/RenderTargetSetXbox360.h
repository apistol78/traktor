#ifndef traktor_render_RenderTargetSetXbox360_H
#define traktor_render_RenderTargetSetXbox360_H

#include "Render/RenderTargetSet.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX9_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX9
 */
class T_DLLCLASS RenderTargetSetXbox360
:	public RenderTargetSet
{
	T_RTTI_CLASS(RenderTargetSetXbox360)

public:
	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual Texture* getColorTexture(int index) const;
};

	}
}

#endif	// traktor_render_RenderTargetSetXbox360_H
