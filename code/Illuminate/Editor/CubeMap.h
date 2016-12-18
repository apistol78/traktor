#ifndef traktor_illuminate_CubeMap_H
#define traktor_illuminate_CubeMap_H

#include "Core/Object.h"

namespace traktor
{
	namespace drawing
	{

class Image;
class PixelFormat;

	}

	namespace illuminate
	{

class CubeMap : public Object
{
	T_RTTI_CLASS;

public:
	CubeMap(int32_t size, const drawing::PixelFormat& pixelFormat);

	CubeMap(const drawing::Image* cubeMap);

	Vector4 getDirection(int32_t side, int32_t x, int32_t y) const;

	void getPosition(const Vector4& direction, int32_t& outSide, int32_t& outX, int32_t& outY) const;

	void set(const Vector4& direction, const Color4f& value);

	Color4f get(const Vector4& direction) const;

	void setSide(int32_t side, drawing::Image* image) { m_side[side] = image; }

	const drawing::Image* getSide(int32_t side) const { return m_side[side]; }

	int32_t getSize() const { return m_size; }

private:
	Ref< drawing::Image > m_side[6];
	int32_t m_size;
};

	}
}

#endif	// traktor_illuminate_CubeMap_H
