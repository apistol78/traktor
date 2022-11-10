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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HTML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace html
	{

/*! HTML DOM Attribute.
 * \ingroup HTML
 */
class T_DLLCLASS Attribute : public Object
{
	T_RTTI_CLASS;

public:
	explicit Attribute(const std::wstring& name, const std::wstring& value = L"");

	const std::wstring& getName() const;

	void setValue(const std::wstring& value);

	const std::wstring& getValue() const;

	Attribute* getPrevious() const;

	Attribute* getNext() const;

private:
	friend class Element;

	std::wstring m_name;
	std::wstring m_value;
	Attribute* m_previous;
	Ref< Attribute > m_next;
};

	}
}

