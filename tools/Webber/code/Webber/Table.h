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
#include <vector>
#include "Webber/ILayout.h"

namespace traktor::wbr
{

/*! Layout which places children row by row in a table, one column per header.
 * \ingroup Webber
 */
class Table : public ILayout
{
	T_RTTI_CLASS;

public:
	explicit Table(const std::vector< std::wstring >& headers);

	virtual void build(const Widget* widget, const RefArray< Widget >& children, html::Element* parent) const override final;

private:
	std::vector< std::wstring > m_headers;
};

}
