/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Matrix33.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

	namespace spark
	{

class ColorTransform;
struct SwfFillStyle;

#if defined (_MSC_VER)
#	pragma warning( disable:4324 )
#endif

/*! Shape fill style.
 * \ingroup Spark
 */
class T_DLLCLASS FillStyle
{
public:
	enum class GradientType
	{
		Invalid = 0,
		Linear = 1,
		Radial = 2
	};

	struct ColorRecord
	{
		float ratio;
		Color4f color;

		ColorRecord() = default;

		ColorRecord(float ratio_, const Color4f& color_)
		:	ratio(ratio_)
		,	color(color_)
		{
		}

		void serialize(ISerializer& s);
	};

	FillStyle();

	bool create(const SwfFillStyle* fillStyle);

	bool create(const Color4f& solidColor);

	bool create(GradientType gradientType, const AlignedVector< ColorRecord >& colorRecords, const Matrix33& gradientMatrix);

	bool create(uint16_t fillBitmap, const Matrix33& fillBitmapMatrix, bool fillBitmapRepeat);

	void transform(const Matrix33& transform, const ColorTransform& cxform);

	const AlignedVector< ColorRecord >& getColorRecords() const { return m_colorRecords; }

	GradientType getGradientType() const { return m_gradientType; }

	const Matrix33& getGradientMatrix() const { return m_gradientMatrix; }

	uint16_t getFillBitmap() const { return m_fillBitmap; }

	const Matrix33& getFillBitmapMatrix() const { return m_fillBitmapMatrix; }

	bool getFillBitmapRepeat() const { return m_fillBitmapRepeat; }

	void serialize(ISerializer& s);

private:
	AlignedVector< ColorRecord > m_colorRecords;
	GradientType m_gradientType;
	Matrix33 m_gradientMatrix;
	uint16_t m_fillBitmap;
	Matrix33 m_fillBitmapMatrix;
	bool m_fillBitmapRepeat;
};

#if defined (_MSC_VER)
#	pragma warning( default:4324 )
#endif

	}
}

