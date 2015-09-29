#ifndef traktor_render_ImageProcessTargetPool_H
#define traktor_render_ImageProcessTargetPool_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"

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
class RenderTargetSet;

/*! \brief Pool of render targets.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessTargetPool : public Object
{
	T_RTTI_CLASS;

public:
	struct Pool
	{
		RenderTargetSetCreateDesc rtscd;
		RefArray< RenderTargetSet > free;
		RefArray< RenderTargetSet > acquired;
	};

	ImageProcessTargetPool(IRenderSystem* renderSystem);

	RenderTargetSet* acquireTarget(const RenderTargetSetCreateDesc& rtscd);

	void releaseTarget(const RenderTargetSetCreateDesc& rtscd, RenderTargetSet* rts);

private:
	Ref< IRenderSystem > m_renderSystem;
	AlignedVector< Pool > m_pool;
};

	}
}

#endif	// traktor_render_ImageProcessTargetPool_H
