/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Webber/Widget.h"

namespace traktor::wbr
{

/*! Markdown text, rendered as HTML.
 *
 * Supports headings, paragraphs, emphasis, inline code, code blocks,
 * lists, block quotes, links, images and horizontal rules. Raw HTML is shown as text.
 *
 * \ingroup Webber
 */
class Markdown : public Widget
{
	T_RTTI_CLASS;

public:
	explicit Markdown(const std::wstring& text);

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	virtual void build(html::Element* parent) const override final;

private:
	std::wstring m_text;
};

}
