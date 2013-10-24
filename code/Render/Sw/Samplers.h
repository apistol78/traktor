#ifndef traktor_render_Samplers_H
#define traktor_render_Samplers_H

#include "Core/Math/Float.h"
#include "Render/Sw/Core/Sampler.h"

namespace traktor
{
	namespace render
	{

/*! \ingroup SW */
//@{

class NullTextureSampler : public AbstractSampler
{
public:
	virtual Vector4 getSize() const
	{
		return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	virtual Vector4 get(const Vector4& texCoord) const
	{
		return Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}
};

struct AddressClamp
{
	static inline int eval(int range, int v)
	{
		if (v < 0) return 0;
		if (v >= range) return range - 1;
		return v;
	}
};

struct AddressWrap
{
	static inline int eval(int range, int v)
	{
		return abs(v % range);
	}
};

class SimpleTextureSw;

template <
	typename AddressU,
	typename AddressV
>
class SimpleTextureSampler : public AbstractSampler
{
public:
	SimpleTextureSampler(SimpleTextureSw* texture)
	:	m_texture(texture)
	,	m_width(m_texture->getWidth())
	,	m_height(m_texture->getHeight())
	{
	}

	inline Vector4 getNearest(int x, int y) const
	{
		const uint32_t* data = m_texture->getData() + x + y * m_width;
		return Vector4(
			((*data & 0x000000ff)      ) / 255.0f,
			((*data & 0x0000ff00) >>  8) / 255.0f,
			((*data & 0x00ff0000) >> 16) / 255.0f,
			((*data & 0xff000000) >> 24) / 255.0f
		);
	}

	virtual Vector4 getSize() const
	{
		return Vector4(float(m_width), float(m_height), 0.0f, 0.0f);
	}

	virtual Vector4 get(const Vector4& texCoord) const
	{
		float tx = !isNan(texCoord.x()) ? texCoord.x() : 0.0f;
		float ty = !isNan(texCoord.y()) ? texCoord.y() : 0.0f;

		tx *= m_width;
		ty *= m_height;

		int ix = int(tx);
		int iy = int(ty);

		int ix1 = AddressU::eval(m_width, ix);
		int iy1 = AddressV::eval(m_height, iy);
		int ix2 = AddressU::eval(m_width, ix + 1);
		int iy2 = AddressV::eval(m_height, iy + 1);

		float fx = tx - ix;
		float fy = ty - iy;

		Vector4 tl = getNearest(ix1, iy1);
		Vector4 tr = getNearest(ix2, iy1);
		Vector4 bl = getNearest(ix1, iy2);
		Vector4 br = getNearest(ix2, iy2);

		Vector4 l = lerp(tl, bl, traktor::Scalar(fy));
		Vector4 r = lerp(tr, br, traktor::Scalar(fy));

		return lerp(l, r, traktor::Scalar(fx));
	}

private:
	Ref< SimpleTextureSw > m_texture;
	int m_width;
	int m_height;
};

class CubeTextureSw;

template <
	typename AddressU,
	typename AddressV,
	typename AddressW
>
class CubeTextureSampler : public AbstractSampler
{
public:
	CubeTextureSampler(CubeTextureSw* texture)
	:	m_texture(texture)
	,	m_side(m_texture->getWidth())
	{
	}

	inline Vector4 getNearest(int x, int y) const
	{
		const uint32_t* data = m_texture->getData() + x + y * m_side;
		return Vector4(
			((*data & 0x000000ff)      ) / 255.0f,
			((*data & 0x0000ff00) >>  8) / 255.0f,
			((*data & 0x00ff0000) >> 16) / 255.0f,
			((*data & 0xff000000) >> 24) / 255.0f
		);
	}

	virtual Vector4 getSize() const
	{
		return Vector4(float(m_side), float(m_side), float(m_side), 0.0f);
	}

	virtual Vector4 get(const Vector4& texCoord) const
	{
		float tx = !isNan(texCoord.x()) ? texCoord.x() : 0.0f;
		float ty = !isNan(texCoord.y()) ? texCoord.y() : 0.0f;
		float tz = !isNan(texCoord.z()) ? texCoord.z() : 0.0f;

		tx *= m_side;
		ty *= m_side;
		tz *= m_side;

		int ix = int(tx);
		int iy = int(ty);
		int iz = int(tz);

		int ix1 = AddressU::eval(m_side, ix);
		int iy1 = AddressV::eval(m_side, iy);
		int ix2 = AddressU::eval(m_side, ix + 1);
		int iy2 = AddressV::eval(m_side, iy + 1);

		float fx = tx - ix;
		float fy = ty - iy;

		Vector4 tl = getNearest(ix1, iy1);
		Vector4 tr = getNearest(ix2, iy1);
		Vector4 bl = getNearest(ix1, iy2);
		Vector4 br = getNearest(ix2, iy2);

		Vector4 l = lerp(tl, bl, traktor::Scalar(fy));
		Vector4 r = lerp(tr, br, traktor::Scalar(fy));

		return lerp(l, r, traktor::Scalar(fx));
	}

private:
	Ref< CubeTextureSw > m_texture;
	int m_side;
};

template <
	typename AddressU,
	typename AddressV
>
class RenderTargetSampler : public AbstractSampler
{
public:
	RenderTargetSampler(RenderTargetSw* renderTarget)
	:	m_renderTarget(renderTarget)
	,	m_width(renderTarget->getWidth())
	,	m_height(renderTarget->getHeight())
	{
	}

	virtual Vector4 getSize() const
	{
		return Vector4(float(m_width), float(m_height), 0.0f, 0.0f);
	}

	virtual Vector4 get(const Vector4& texCoord) const
	{
		int x = int(m_width * texCoord.x() + 0.5f);
		int y = int(m_height * texCoord.y() + 0.5f);

		x = AddressU::eval(m_width, x);
		y = AddressV::eval(m_height, y);

		const uint16_t* data = m_renderTarget->getColorSurface() + x + y * m_width;

		return Vector4(
			((*data & 0xf800) >> 11) / float((1 << 5) - 1),
			((*data & 0x07e0) >> 5) / float((1 << 6) - 1),
			 (*data & 0x001f) / float((1 << 5) - 1),
			0.0f
		);
	}

private:
	Ref< RenderTargetSw > m_renderTarget;
	int m_width;
	int m_height;
};

//@}

	}
}

#endif	// traktor_render_Samplers_H
