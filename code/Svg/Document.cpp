/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Svg/Document.h"

namespace traktor::svg
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.Document", Document, Shape)

Document::Document()
:	m_size(0.0f, 0.0f)
{
}

void Document::setViewBox(const Aabb2& viewBox)
{
	m_viewBox = viewBox;
}

const Aabb2& Document::getViewBox() const
{
	return m_viewBox;
}

void Document::setSize(const Vector2& size)
{
	m_size = size;
}

const Vector2& Document::getSize() const
{
	return m_size;
}

}
