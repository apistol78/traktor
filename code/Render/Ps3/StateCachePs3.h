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

	void setProgram(const CGprogram vertexProgram, const void* vertexUCode, const CGprogram fragmentProgram, const uint32_t fragmentOffset);

	void reset(bool force);

private:
	bool m_inFp32Mode;
	RenderState m_renderState;
	const void* m_vertexUCode;
	uint32_t m_fragmentOffset;
};

	}
}

#endif	// traktor_render_StateCachePs3_H
