#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Flash/FlashFillStyle.h"
#include "Flash/SwfMembers.h"

namespace traktor
{
	namespace flash
	{

FlashFillStyle::FlashFillStyle()
:	m_gradientType(GtInvalid)
,	m_gradientMatrix(Matrix33::identity())
,	m_fillBitmap(0)
,	m_fillBitmapMatrix(Matrix33::identity())
,	m_fillBitmapRepeat(false)
{
}

bool FlashFillStyle::create(const SwfFillStyle* fillStyle)
{
	// Create solid white if null descriptor given.
	if (!fillStyle)
	{
		SwfColor dummy = { 255, 255, 255, 255 };
		m_colorRecords.push_back(ColorRecord(0.0f, dummy));
		return true;
	}

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
		m_fillBitmapRepeat = bool(fillStyle->type == FstTiledBitmap || fillStyle->type == FstTiledBitmapHard);
	}
	else
		m_fillBitmap = 0;

	return true;
}

bool FlashFillStyle::create(const SwfColor& solidColor)
{
	m_colorRecords.push_back(ColorRecord(0.0f, solidColor));
	return true;
}

bool FlashFillStyle::create(GradientType gradientType, const AlignedVector< ColorRecord >& colorRecords, const Matrix33& gradientMatrix)
{
	m_gradientType = gradientType;
	m_colorRecords = colorRecords;
	m_gradientMatrix = gradientMatrix;
	return true;
}

bool FlashFillStyle::create(uint16_t fillBitmap, const Matrix33& fillBitmapMatrix, bool fillBitmapRepeat)
{
	m_fillBitmap = fillBitmap;
	m_fillBitmapMatrix = fillBitmapMatrix;
	m_fillBitmapRepeat = fillBitmapRepeat;
	return true;
}

void FlashFillStyle::transform(const Matrix33& transform, const SwfCxTransform& cxform)
{
	for (AlignedVector< ColorRecord >::iterator i = m_colorRecords.begin(); i != m_colorRecords.end(); ++i)
	{
		i->color.red =   uint8_t(((i->color.red   / 255.0f) * cxform.red[0]   + cxform.red[1]  ) * 255.0f);
		i->color.green = uint8_t(((i->color.green / 255.0f) * cxform.green[0] + cxform.green[1]) * 255.0f);
		i->color.blue =  uint8_t(((i->color.blue  / 255.0f) * cxform.blue[0]  + cxform.blue[1] ) * 255.0f);
		i->color.alpha = uint8_t(((i->color.alpha / 255.0f) * cxform.alpha[0] + cxform.alpha[1]) * 255.0f);
	}

	if (m_gradientType != GtInvalid)
		m_gradientMatrix = transform * m_gradientMatrix;

	if (m_fillBitmap != 0)
		m_fillBitmapMatrix = transform * m_fillBitmapMatrix;
}

void FlashFillStyle::serialize(ISerializer& s)
{
	const MemberEnum< GradientType >::Key kGradientType[] =
	{
		{ L"GtInvalid", GtInvalid },
		{ L"GtLinear", GtLinear },
		{ L"GtRadial", GtRadial },
		{ 0, 0 }
	};

	s >> MemberAlignedVector< ColorRecord, MemberComposite< ColorRecord > >(L"colorRecords", m_colorRecords);
	s >> MemberEnum< GradientType >(L"gradientType", m_gradientType, kGradientType);
	s >> Member< Matrix33 >(L"gradientMatrix", m_gradientMatrix);
	s >> Member< uint16_t >(L"m_fillBitmap", m_fillBitmap);
	s >> Member< Matrix33 >(L"fillBitmapMatrix", m_fillBitmapMatrix);
	s >> Member< bool >(L"fillBitmapRepeat", m_fillBitmapRepeat);
}

void FlashFillStyle::ColorRecord::serialize(ISerializer& s)
{
	s >> Member< float >(L"ratio", ratio);
	s >> MemberSwfColor(L"color", color);
}

	}
}
