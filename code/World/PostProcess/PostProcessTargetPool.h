#ifndef traktor_world_PostProcessTargetPool_H
#define traktor_world_PostProcessTargetPool_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class RenderTargetSet;

	}

	namespace world
	{

/*! \brief Pool of render targets.
 * \ingroup World
 */
class PostProcessTargetPool : public Object
{
	T_RTTI_CLASS;

public:
	struct Pool
	{
		render::RenderTargetSetCreateDesc rtscd;
		RefArray< render::RenderTargetSet > free;
		RefArray< render::RenderTargetSet > acquired;
	};

	PostProcessTargetPool(render::IRenderSystem* renderSystem);

	render::RenderTargetSet* acquireTarget(const render::RenderTargetSetCreateDesc& rtscd);

	void releaseTarget(const render::RenderTargetSetCreateDesc& rtscd, render::RenderTargetSet* rts);

private:
	Ref< render::IRenderSystem > m_renderSystem;
	AlignedVector< Pool > m_pool;
};

	}
}

#endif	// traktor_world_PostProcessTargetPool_H
