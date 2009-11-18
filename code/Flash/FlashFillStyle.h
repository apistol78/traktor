#ifndef traktor_flash_FlashFillStyle_H
#define traktor_flash_FlashFillStyle_H

#include "Core/Object.h"
#include "Core/Math/Matrix33.h"
#include "Core/Containers/AlignedVector.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief Shape fill style.
 * \ingroup Flash
 */
class T_DLLCLASS FlashFillStyle : public Object
{
	T_RTTI_CLASS;

public:
	enum GradientType
	{
		GtInvalid,
		GtLinear,
		GtRadial
	};

	struct ColorRecord
	{
		float ratio;
		SwfColor color;

		ColorRecord()
		{
		}

		ColorRecord(float ratio_, const SwfColor color_)
		:	ratio(ratio_)
		,	color(color_)
		{
		}
	};

	FlashFillStyle();

	bool create(const SwfFillStyle* fillStyle);

	const AlignedVector< ColorRecord >& getColorRecords() const { return m_colorRecords; }

	GradientType getGradientType() const { return m_gradientType; }

	const Matrix33& getGradientMatrix() const { return m_gradientMatrix; }

	uint16_t getFillBitmap() const { return m_fillBitmap; }

	const Matrix33& getFillBitmapMatrix() const { return m_fillBitmapMatrix; }

private:
	AlignedVector< ColorRecord > m_colorRecords;
	GradientType m_gradientType;
	Matrix33 m_gradientMatrix;
	uint16_t m_fillBitmap;
	Matrix33 m_fillBitmapMatrix;
};

	}
}

#endif	// traktor_flash_FlashFillStyle_H
