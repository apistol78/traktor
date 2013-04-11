#ifndef traktor_terrain_SmoothBrush_H
#define traktor_terrain_SmoothBrush_H

#include "Resource/Proxy.h"
#include "Terrain/Editor/IBrush.h"

namespace traktor
{
	namespace hf
	{

class Heightfield;

	}

	namespace terrain
	{

class SmoothBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	SmoothBrush(const resource::Proxy< hf::Heightfield >& heightfield);

	virtual void begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength);

	virtual void apply(int32_t x, int32_t y);

	virtual void end(int32_t x, int32_t y);

private:
	resource::Proxy< hf::Heightfield > m_heightfield;
	int32_t m_radius;
	const IFallOff* m_fallOff;
	float m_strength;
};

	}
}

#endif	// traktor_terrain_SmoothBrush_H
