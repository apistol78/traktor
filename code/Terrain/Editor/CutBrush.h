#ifndef traktor_terrain_CutBrush_H
#define traktor_terrain_CutBrush_H

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

class CutBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	CutBrush(const resource::Proxy< hf::Heightfield >& heightfield);

	virtual uint32_t begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color);

	virtual void apply(int32_t x, int32_t y);

	virtual void end(int32_t x, int32_t y);

private:
	resource::Proxy< hf::Heightfield > m_heightfield;
	int32_t m_radius;
	bool m_cut;
};

	}
}

#endif	// traktor_terrain_CutBrush_H
