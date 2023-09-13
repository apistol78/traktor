/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Color4f.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SVG_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::svg
{

class Gradient;
class Shape;

/*! SVG style description.
 * \ingroup SVG
 */
class T_DLLCLASS Style : public Object
{
	T_RTTI_CLASS;

public:
	Style();

	void setFillEnable(bool fillEnable);

	bool getFillEnable() const;

	void setFillGradient(const Gradient* fillGradient);

	const Gradient* getFillGradient() const;

	void setFill(const Color4f& fill);

	const Color4f& getFill() const;

	void setStrokeEnable(bool strokeEnable);

	bool getStrokeEnable() const;

	void setStrokeGradient(const Gradient* strokeGradient);

	const Gradient* getStrokeGradient() const;

	void setStrokeWidth(float strokeWidth);

	float getStrokeWidth() const;

	void setStroke(const Color4f& stroke);

	const Color4f& getStroke() const;

	void setOpacity(float opacity);

	float getOpacity() const;

	void setFontFamily(const std::wstring& fontFamily);

	const std::wstring& getFontFamily() const;

	void setFontSize(float fontSize);

	float getFontSize() const;

	void setShapeInside(const Shape* shape);

	const Shape* getShapeInside() const;

	bool operator == (const Style& other) const;

private:
	bool m_fillEnable;
	Ref< const Gradient > m_fillGradient;
	Color4f m_fill;
	bool m_strokeEnable;
	Ref< const Gradient > m_strokeGradient;
	float m_strokeWidth;
	Color4f m_stroke;
	float m_opacity;
	std::wstring m_fontFamily;
	float m_fontSize;
	Ref< const Shape > m_shapeInside;
};

}
