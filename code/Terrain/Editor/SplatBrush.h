#pragma once

#include "Core/Math/Color4f.h"
#include "Resource/Proxy.h"
#include "Terrain/Editor/IBrush.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace hf
	{

class Heightfield;

	}

	namespace terrain
	{

class SplatBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	SplatBrush(const resource::Proxy< hf::Heightfield >& heightfield, drawing::Image* splatImage);

	virtual uint32_t begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material) override final;

	virtual void apply(int32_t x, int32_t y) override final;

	virtual void end(int32_t x, int32_t y) override final;

private:
	resource::Proxy< hf::Heightfield > m_heightfield;
	Ref< drawing::Image > m_splatImage;
	int32_t m_radius;
	const IFallOff* m_fallOff;
	float m_strength;
	int32_t m_material;
	bool m_inverse;
};

	}
}

