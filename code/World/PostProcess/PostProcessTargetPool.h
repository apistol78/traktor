#ifndef traktor_world_PostProcessTargetPool_H
#define traktor_world_PostProcessTargetPool_H

#include <map>
#include "Core/Object.h"
#include "Core/RefArray.h"
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
	PostProcessTargetPool(render::IRenderSystem* renderSystem);

	render::RenderTargetSet* acquireTarget(const render::RenderTargetSetCreateDesc& rtscd);

	void releaseTarget(const render::RenderTargetSetCreateDesc& rtscd, render::RenderTargetSet* rts);

private:
	struct Pool
	{
		render::RenderTargetSetCreateDesc rtscd;
		RefArray< render::RenderTargetSet > free;
		RefArray< render::RenderTargetSet > acquired;
	};

	Ref< render::IRenderSystem > m_renderSystem;
	std::map< uint32_t, Pool > m_pool;
};

	}
}

#endif	// traktor_world_PostProcessTargetPool_H
