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

	void setInFp32Mode(bool inFp32Mode);

	void setRenderState(const RenderState& rs);

	void reset(bool force);

private:
	bool m_inFp32Mode;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_StateCachePs3_H
