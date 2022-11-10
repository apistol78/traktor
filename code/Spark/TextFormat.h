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
#include "Spark/Swf/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class T_DLLCLASS TextFormat : public Object
{
	T_RTTI_CLASS;

public:
	explicit TextFormat(float letterSpacing, SwfTextAlignType align, float size);

	void setLetterSpacing(float letterSpacing) { m_letterSpacing = letterSpacing; }

	float getLetterSpacing() const { return m_letterSpacing; }

	void setAlign(SwfTextAlignType align) { m_align = align; }

	SwfTextAlignType getAlign() const { return m_align; }

	void setSize(float size) { m_size = size; }

	float getSize() const { return m_size; }

private:
	float m_letterSpacing;
	SwfTextAlignType m_align;
	float m_size;
};

	}
}

