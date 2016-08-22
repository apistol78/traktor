#ifndef traktor_render_IVRCompositor_H
#define traktor_render_IVRCompositor_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class IRenderView;

/*! \brief VR compositor interface.
 * \ingroup Render
 */
class T_DLLCLASS IVRCompositor : public Object
{
	T_RTTI_CLASS;
	
public:
	virtual bool create(IRenderSystem* renderSystem, IRenderView* renderView) = 0;

	virtual void destroy() = 0;

	virtual bool beginRenderEye(IRenderView* renderView, int32_t eye) = 0;

	virtual bool endRenderEye(IRenderView* renderView, int32_t eye) = 0;

	virtual bool presentCompositeOutput(IRenderView* renderView) = 0;	
};

	}
}

#endif	// traktor_render_IVRCompositor_H
