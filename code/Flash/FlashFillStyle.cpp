#include "Flash/FlashFillStyle.h"

namespace traktor
{
	namespace flash
	{

FlashFillStyle::FlashFillStyle()
:	m_gradientType(GtInvalid)
,	m_gradientMatrix(Matrix33::identity())
,	m_fillBitmap(0)
,	m_fillBitmapMatrix(Matrix33::identity())
{
}

bool FlashFillStyle::create(const SwfFillStyle* fillStyle)
{
	if (fillStyle->type == FstSolid)
		m_colorRecords.push_back(ColorRecord(0.0f, fillStyle->solid.color));
	else if (fillStyle->type == FstLinearGradient || fillStyle->type == FstRadialGradient)
	{
		m_colorRecords.reserve(fillStyle->gradient.gradient->numGradientRecords);
		for (uint8_t i = 0; i < fillStyle->gradient.gradient->numGradientRecords; ++i)
		{
			m_colorRecords.push_back(ColorRecord(
				fillStyle->gradient.gradient->gradientRecords[i]->ratio / 255.0f,
				fillStyle->gradient.gradient->gradientRecords[i]->color
			));
		}

		if (fillStyle->type == FstLinearGradient)
			m_gradientType = GtLinear;
		else
			m_gradientType = GtRadial;

		m_gradientMatrix = Matrix33(fillStyle->gradient.gradientMatrix.m);
	}
	else
	{
		SwfColor dummy = { 255, 255, 255, 255 };
		m_colorRecords.push_back(ColorRecord(0.0f, dummy));
	}
	
	if (
		fillStyle->type == FstTiledBitmap || fillStyle->type == FstClippedBitmap ||
		fillStyle->type == FstTiledBitmapHard || fillStyle->type == FstClippedBitmapHard
	)
	{
		m_fillBitmap = fillStyle->bitmap.bitmapId; 
		m_fillBitmapMatrix = Matrix33(fillStyle->bitmap.bitmapMatrix.m);
	}
	else
		m_fillBitmap = 0;

	return true;
}

	}
}
