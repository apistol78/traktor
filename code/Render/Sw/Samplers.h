/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Samplers_H
#define traktor_render_Samplers_H

#include "Core/Ref.h"
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

class VolumeTextureSw;

template <
	typename AddressU,
	typename AddressV,
	typename AddressW
>
class VolumeTextureSampler : public AbstractSampler
{
public:
	VolumeTextureSampler(VolumeTextureSw* texture)
	:	m_texture(texture)
	,	m_width(m_texture->getWidth())
	,	m_height(m_texture->getHeight())
	,	m_depth(m_texture->getDepth())
	{
	}

	inline Vector4 getNearest(int x, int y, int z) const
	{
		const uint32_t* data = m_texture->getData() + x + y * m_width + z * m_width * m_height;
		return Vector4(
			((*data & 0x000000ff)      ) / 255.0f,
			((*data & 0x0000ff00) >>  8) / 255.0f,
			((*data & 0x00ff0000) >> 16) / 255.0f,
			((*data & 0xff000000) >> 24) / 255.0f
		);
	}

	virtual Vector4 getSize() const
	{
		return Vector4(float(m_width), float(m_height), float(m_depth), 0.0f);
	}

	virtual Vector4 get(const Vector4& texCoord) const
	{
		float tx = !isNan(texCoord.x()) ? texCoord.x() : 0.0f;
		float ty = !isNan(texCoord.y()) ? texCoord.y() : 0.0f;
		float tz = !isNan(texCoord.z()) ? texCoord.z() : 0.0f;

		tx *= m_width;
		ty *= m_height;
		tz *= m_depth;

		int ix = int(tx);
		int iy = int(ty);
		int iz = int(tz);

		int ix1 = AddressU::eval(m_width, ix);
		int iy1 = AddressV::eval(m_height, iy);
		int iz1 = AddressW::eval(m_depth, iz);
		int ix2 = AddressU::eval(m_width, ix + 1);
		int iy2 = AddressV::eval(m_height, iy + 1);
		int iz2 = AddressW::eval(m_depth, iz + 1);

		float fx = tx - ix;
		float fy = ty - iy;

		Vector4 tl = getNearest(ix1, iy1, iz1);
		Vector4 tr = getNearest(ix2, iy1, iz1);
		Vector4 bl = getNearest(ix1, iy2, iz1);
		Vector4 br = getNearest(ix2, iy2, iz1);

		Vector4 l = lerp(tl, bl, traktor::Scalar(fy));
		Vector4 r = lerp(tr, br, traktor::Scalar(fy));

		return lerp(l, r, traktor::Scalar(fx));
	}

private:
	Ref< VolumeTextureSw > m_texture;
	int m_width;
	int m_height;
	int m_depth;
};

//@}

	}
}

#endif	// traktor_render_Samplers_H
