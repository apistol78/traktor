#ifndef traktor_terrain_SymmetricalBrush_H
#define traktor_terrain_SymmetricalBrush_H

#include "Terrain/Editor/IBrush.h"

namespace traktor
{
	namespace terrain
	{

class SymmetricalBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	SymmetricalBrush(const int32_t scale[2], const int32_t offset[2], IBrush* brush);

	virtual uint32_t begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material);

	virtual void apply(int32_t x, int32_t y);

	virtual void end(int32_t x, int32_t y);

	virtual Ref< IBrush > clone() const;

	virtual bool contained() const { return false; }

private:
	int32_t m_scale[2];
	int32_t m_offset[2];
	Ref< IBrush > m_brush[2];
};

	}
}

#endif	// traktor_terrain_SymmetricalBrush_H
