#ifndef traktor_drawing_Raster_H
#define traktor_drawing_Raster_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix33.h"

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

	enum FillRuleType
	{
		FrOddEven,
		FrNonZero
	};

	Raster();

	Raster(Image* image);

	bool valid() const;

	bool setImage(Image* image);

	void setMask(Image* image);

	void clearStyles();

	int32_t defineSolidStyle(const Color4f& color);

	int32_t defineLinearGradientStyle(const Matrix33& gradientMatrix, const AlignedVector< std::pair< Color4f, float > >& colors);

	int32_t defineRadialGradientStyle(const Matrix33& gradientMatrix, const AlignedVector< std::pair< Color4f, float > >& colors);

	int32_t defineImageStyle(const Matrix33& imageMatrix, const Image* image, bool repeat);

	void clear();

	void moveTo(const Vector2& p) { moveTo(p.x, p.y); }

	void moveTo(float x, float y);

	void lineTo(const Vector2& p) { lineTo(p.x, p.y); }

	void lineTo(float x, float y);

	void quadricTo(const Vector2& p, const Vector2& c) { quadricTo(p.x, p.y, c.x, c.y); }

	void quadricTo(float x1, float y1, float x, float y);

	void quadricTo(const Vector2& p) { quadricTo(p.x, p.y); }

	void quadricTo(float x, float y);

	void cubicTo(const Vector2& p1, const Vector2& p2, const Vector2& c) { cubicTo(p1.x, p1.y, p2.x, p2.y, c.x, c.y); }

	void cubicTo(float x1, float y1, float x2, float y2, float x, float y);

	void cubicTo(const Vector2& p2, const Vector2& c) { cubicTo(p2.x, p2.y, c.x, c.y); }

	void cubicTo(float x2, float y2, float x, float y);

	void close();

	void rect(float x, float y, float width, float height, float radius);

	void circle(float x, float y, float radius);

	void fill(int32_t style0, int32_t style1, FillRuleType fillRule);

	void stroke(int32_t style, float width, StrokeCapType cap);

	void submit();

private:
	Ref< IRasterImpl > m_impl;
};

	}
}

#endif	// traktor_drawing_Raster_H
