/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Spark/ColorTransform.h"
#include "Spark/FillStyle.h"
#include "Spark/Swf/SwfTypes.h"

namespace traktor
{
	namespace spark
	{

FillStyle::FillStyle()
:	m_gradientType(GradientType::Invalid)
,	m_gradientMatrix(Matrix33::identity())
,	m_fillBitmap(0)
,	m_fillBitmapMatrix(Matrix33::identity())
,	m_fillBitmapRepeat(false)
{
}

bool FillStyle::create(const SwfFillStyle* fillStyle)
{
	// Create solid white if null descriptor given.
	if (!fillStyle)
	{
		const Color4f dummy(1.0f, 1.0f, 1.0f, 1.0f);
		m_colorRecords.push_back(ColorRecord(0.0f, dummy));
		return true;
	}

	if (fillStyle->type == FstSolid)
		m_colorRecords.push_back(ColorRecord(0.0f, Color4f::loadUnaligned(fillStyle->solid.color)));
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
			m_gradientType = GradientType::Linear;
		else
			m_gradientType = GradientType::Radial;

		m_gradientMatrix = Matrix33(fillStyle->gradient.gradientMatrix.m);
	}
	else
	{
		const Color4f dummy(1.0f, 1.0f, 1.0f, 1.0f);
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

bool FillStyle::create(const Color4f& solidColor)
{
	m_colorRecords.push_back(ColorRecord(0.0f, solidColor));
	return true;
}

bool FillStyle::create(GradientType gradientType, const AlignedVector< ColorRecord >& colorRecords, const Matrix33& gradientMatrix)
{
	m_gradientType = gradientType;
	m_colorRecords = colorRecords;
	m_gradientMatrix = gradientMatrix;
	return true;
}

bool FillStyle::create(uint16_t fillBitmap, const Matrix33& fillBitmapMatrix, bool fillBitmapRepeat)
{
	m_fillBitmap = fillBitmap;
	m_fillBitmapMatrix = fillBitmapMatrix;
	m_fillBitmapRepeat = fillBitmapRepeat;
	return true;
}

void FillStyle::transform(const Matrix33& transform, const ColorTransform& cxform)
{
	for (AlignedVector< ColorRecord >::iterator i = m_colorRecords.begin(); i != m_colorRecords.end(); ++i)
		i->color = i->color * cxform.mul + cxform.add;

	if (m_gradientType != GradientType::Invalid)
		m_gradientMatrix = transform * m_gradientMatrix;

	if (m_fillBitmap != 0)
		m_fillBitmapMatrix = transform * m_fillBitmapMatrix;
}

void FillStyle::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< ColorRecord, MemberComposite< ColorRecord > >(L"colorRecords", m_colorRecords);
	s >> MemberEnumByValue< GradientType >(L"gradientType", m_gradientType);
	s >> Member< Matrix33 >(L"gradientMatrix", m_gradientMatrix);
	s >> Member< uint16_t >(L"m_fillBitmap", m_fillBitmap);
	s >> Member< Matrix33 >(L"fillBitmapMatrix", m_fillBitmapMatrix);
	s >> Member< bool >(L"fillBitmapRepeat", m_fillBitmapRepeat);
}

void FillStyle::ColorRecord::serialize(ISerializer& s)
{
	s >> Member< float >(L"ratio", ratio);
	s >> Member< Color4f >(L"color", color);
}

	}
}
