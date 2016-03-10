#ifndef traktor_drawing_Raster_H
#define traktor_drawing_Raster_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Color4f;

	namespace drawing
	{

class Image;

/*! \brief Raster primitives.
 * \ingroup Drawing
 */
class T_DLLCLASS Raster : public Object
{
	T_RTTI_CLASS;

public:
	Raster(Image* image);

	void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color4f& color);

	void drawPixel(int32_t x, int32_t y, const Color4f& color);

	void drawPixel(int32_t x, int32_t y, const Color4f& color, float alpha);

	void drawCircle(int32_t x, int32_t y, int32_t radius, const Color4f& color);

	void drawFilledCircle(int32_t x, int32_t y, int32_t radius, const Color4f& color);

	void drawRectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const Color4f& color);

	void drawFilledRectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const Color4f& color);

	void drawPolygon(const Vector2* points, uint32_t npoints, const Color4f& color);

	void drawPolyLine(const Vector2* points, uint32_t npoints, const Color4f& color);

private:
	struct Span
	{
		float x;
		int8_t fillDelta;
	};

	typedef AlignedVector< Span > spanline_t;

	Ref< Image > m_image;
	AlignedVector< Raster::spanline_t > m_spanlines;

	void insertSpan(spanline_t& spanline, const Span& span) const;
};

	}
}

#endif	// traktor_drawing_Raster_H
