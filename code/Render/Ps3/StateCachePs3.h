#ifndef traktor_render_StateCachePs3_H
#define traktor_render_StateCachePs3_H

#include "Render/Ps3/TypesPs3.h"

namespace traktor
{
	namespace render
	{

class StateCachePs3
{
public:
	StateCachePs3();

	void setRenderState(const RenderState& rs);

	void reset();

private:
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_StateCachePs3_H
