#ifndef traktor_terrain_MaterialBrush_H
#define traktor_terrain_MaterialBrush_H

#include "Core/Math/Color4f.h"
#include "Terrain/Editor/IBrush.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace terrain
	{

class MaterialBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	MaterialBrush(drawing::Image* splatImage);

	virtual uint32_t begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material);

	virtual void apply(int32_t x, int32_t y);

	virtual void end(int32_t x, int32_t y);

	virtual Ref< IBrush > clone() const;

	virtual bool contained() const { return true; }

private:
	Ref< drawing::Image > m_splatImage;
	int32_t m_radius;
	const IFallOff* m_fallOff;
	float m_strength;
	int32_t m_material;
};

	}
}

#endif	// traktor_terrain_MaterialBrush_H
