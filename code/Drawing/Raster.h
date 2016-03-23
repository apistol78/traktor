#ifndef traktor_drawing_Raster_H
#define traktor_drawing_Raster_H

#include "Core/Object.h"
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
class IRasterImpl;

/*! \brief Raster primitives.
 * \ingroup Drawing
 */
class T_DLLCLASS Raster : public Object
{
	T_RTTI_CLASS;

public:
	enum StrokeCapType
	{
		ScButt,
		ScSquare,
		ScRound
	};

	Raster(Image* image);

	bool valid() const;

	void clear();

	void moveTo(float x, float y);

	void lineTo(float x, float y);

	void quadricTo(float x1, float y1, float x, float y);

	void quadricTo(float x, float y);

	void cubicTo(float x1, float y1, float x2, float y2, float x, float y);

	void cubicTo(float x2, float y2, float x, float y);

	void close();

	void rect(float x, float y, float width, float height, float radius);

	void circle(float x, float y, float radius);

	void fill(const Color4f& color);

	void stroke(const Color4f& color, float width, StrokeCapType cap);

private:
	Ref< IRasterImpl > m_impl;
};

	}
}

#endif	// traktor_drawing_Raster_H
