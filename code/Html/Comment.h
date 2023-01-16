/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Html/Node.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HTML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::html
{

/*! HTML DOM Comment.
 * \ingroup HTML
 */
class T_DLLCLASS Comment : public Node
{
	T_RTTI_CLASS;

public:
	explicit Comment(const std::wstring& text);

	virtual std::wstring getName() const override final;

	virtual std::wstring getValue() const override final;

	virtual void toString(OutputStream& os) const override final;

private:
	std::wstring m_text;
};

}
