/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Core/Math/Half.h"
#include "Render/Sw/Samplers.h"
#include "Render/Sw/SimpleTextureSw.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

template <
	typename AddressU,
	typename AddressV
>
class SimpleTextureSampler_TfR8 : public AbstractSampler
{
public:
	SimpleTextureSampler_TfR8(const SimpleTextureSw* texture, const uint8_t* data)
	:	m_texture(texture)
	,	m_data(data)
	,	m_width(m_texture->getWidth())
	,	m_height(m_texture->getHeight())
	{
	}

	inline Vector4 getNearest(int x, int y) const
	{
		float sample = m_data[x + y * m_width] / 255.0f;
		return Vector4(sample, sample, sample, sample);
	}

	virtual Vector4 getSize() const T_OVERRIDE T_FINAL
	{
		return Vector4(float(m_width), float(m_height), 0.0f, 0.0f);
	}

	virtual Vector4 get(const Vector4& texCoord) const T_OVERRIDE T_FINAL
	{
		float tx = texCoord.x();
		float ty = texCoord.y();

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
	Ref< const SimpleTextureSw > m_texture;
	const uint8_t* m_data;
	int m_width;
	int m_height;
};

template <
	typename AddressU,
	typename AddressV
>
class SimpleTextureSampler_TfR8G8B8A8 : public AbstractSampler
{
public:
	SimpleTextureSampler_TfR8G8B8A8(const SimpleTextureSw* texture, const uint8_t* data)
	:	m_texture(texture)
	,	m_data(data)
	,	m_width(m_texture->getWidth())
	,	m_height(m_texture->getHeight())
	{
	}

	inline Vector4 getNearest(int x, int y) const
	{
		const uint8_t* data = m_data + (x + y * m_width) * 4;
		return Vector4(
			data[0] / 255.0f,
			data[1] / 255.0f,
			data[2] / 255.0f,
			data[3] / 255.0f
		);
	}

	virtual Vector4 getSize() const
	{
		return Vector4(float(m_width), float(m_height), 0.0f, 0.0f);
	}

	virtual Vector4 get(const Vector4& texCoord) const
	{
		float tx = texCoord.x();
		float ty = texCoord.y();

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
	Ref< const SimpleTextureSw > m_texture;
	const uint8_t* m_data;
	int m_width;
	int m_height;
};

template <
	typename AddressU,
	typename AddressV
>
class SimpleTextureSampler_TfR16F : public AbstractSampler
{
public:
	SimpleTextureSampler_TfR16F(const SimpleTextureSw* texture, const half_t* data)
	:	m_texture(texture)
	,	m_data(data)
	,	m_width(m_texture->getWidth())
	,	m_height(m_texture->getHeight())
	{
	}

	inline Vector4 getNearest(int x, int y) const
	{
		float sample = halfToFloat(m_data[x + y * m_width]);
		return Vector4(sample, sample, sample, sample);
	}

	virtual Vector4 getSize() const
	{
		return Vector4(float(m_width), float(m_height), 0.0f, 0.0f);
	}

	virtual Vector4 get(const Vector4& texCoord) const
	{
		float tx = texCoord.x();
		float ty = texCoord.y();

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
	Ref< const SimpleTextureSw > m_texture;
	const half_t* m_data;
	int m_width;
	int m_height;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureSw", SimpleTextureSw, ISimpleTexture)

SimpleTextureSw::SimpleTextureSw()
:	m_width(0)
,	m_height(0)
,	m_data(0)
,	m_lock(0)
{
}

SimpleTextureSw::~SimpleTextureSw()
{
	destroy();
}

bool SimpleTextureSw::create(const SimpleTextureCreateDesc& desc)
{
	if (desc.format >= TfDXT1)
		return false;

	uint32_t blockSize = getTextureBlockSize(desc.format);

	m_width = desc.width;
	m_height = desc.height;
	m_format = desc.format;
	m_data = new uint8_t [m_width * m_height * blockSize];
	T_ASSERT (m_data);

	if (desc.immutable)
	{
		std::memcpy(
			m_data,
			desc.initialData[0].data,
			m_width * m_height * blockSize
		);
	}

	return true;
}

void SimpleTextureSw::destroy()
{
	T_ASSERT_M (!m_lock, L"Texture still locked");
	delete[] m_data;
	m_data = 0;
}

ITexture* SimpleTextureSw::resolve()
{
	return this;
}

int SimpleTextureSw::getWidth() const
{
	return m_width;
}

int SimpleTextureSw::getHeight() const
{
	return m_height;
}

bool SimpleTextureSw::lock(int level, Lock& lock)
{
	if (m_lock)
		return false;

	m_lock = new uint8_t [m_width * m_height * getTextureBlockSize(m_format)];
	
	lock.pitch = m_width * getTextureBlockSize(m_format);
	lock.bits = m_lock;

	return true;
}

void SimpleTextureSw::unlock(int level)
{
	if (!m_lock)
		return;

	if (level == 0)
	{
		if (getTextureBlockSize(m_format) == 4)
			std::memcpy(m_data, m_lock, m_width * m_height * sizeof(uint32_t));
		else
			T_BREAKPOINT;
	}

	delete[] m_lock;
	m_lock = 0;
}

void* SimpleTextureSw::getInternalHandle()
{
	return 0;
}

Ref< AbstractSampler > SimpleTextureSw::createSampler() const
{
	switch (m_format)
	{
	case TfR8:
		return new SimpleTextureSampler_TfR8< AddressWrap, AddressWrap >(this, m_data);

	case TfR8G8B8A8:
		return new SimpleTextureSampler_TfR8G8B8A8< AddressWrap, AddressWrap >(this, m_data);

	case TfR16F:
		return new SimpleTextureSampler_TfR16F< AddressWrap, AddressWrap >(this, (const half_t*)m_data);

	default:
		return 0;
	}
}

	}
}
