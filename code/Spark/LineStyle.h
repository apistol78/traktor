/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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
struct SwfLineStyle;

/*! Shape line style.
 * \ingroup Spark
 */
class T_DLLCLASS LineStyle
{
public:
	LineStyle();

	bool create(const SwfLineStyle* lineStyle);

	bool create(const Color4f& lineColor, uint16_t lineWidth);

	void transform(const ColorTransform& cxform);

	const Color4f& getLineColor() const;

	uint16_t getLineWidth() const;

	void serialize(ISerializer& s);

private:
	Color4f m_lineColor;
	uint16_t m_lineWidth;
};

	}
}

