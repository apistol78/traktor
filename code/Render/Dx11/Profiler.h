#ifndef traktor_render_Profiler_H
#define traktor_render_Profiler_H

#include "Core/Config.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

class Profiler
{
public:
	Profiler();
	
	bool create(ID3D11Device* d3dDevice);

	void begin(ID3D11DeviceContext* d3dContext);

	void end(ID3D11DeviceContext* d3dContext);

	uint64_t get() const { return m_us; }

private:
	enum { QueryCount = 4 };
	ComRef< ID3D11Query > m_timeQueries[QueryCount * 2];
	ComRef< ID3D11Query > m_disjointQueries[QueryCount];
	int32_t m_count;
	uint64_t m_us;
};

	}
}

#endif	// traktor_render_Profiler_H
