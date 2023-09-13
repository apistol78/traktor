/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Svg/Style.h"
#include "Svg/TextShape.h"

namespace traktor::svg
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.TextShape", TextShape, Shape)

TextShape::TextShape(const Vector2& position, const std::wstring& text)
:	m_position(position)
,	m_text(text)
{
}

const Vector2& TextShape::getPosition() const
{
	return m_position;
}

const std::wstring& TextShape::getText() const
{
	return m_text;
}

Aabb2 TextShape::getBoundingBox() const
{
	const Style* style = getStyle();
	if (style != nullptr && style->getShapeInside() != nullptr)
		return getTransform() * style->getShapeInside()->getBoundingBox();
	else
		return Aabb2();
}

}
