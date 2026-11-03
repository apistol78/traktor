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

/*! Image; source is resolved through the application.
 * \ingroup Webber
 */
class Image : public Widget
{
	T_RTTI_CLASS;

public:
	/*! \param text Alternative text, shown when the image cannot be displayed. */
	explicit Image(const std::wstring& source, const std::wstring& text);

	virtual void build(html::Element* parent) const override final;

private:
	std::wstring m_source;
	std::wstring m_text;
};

}
