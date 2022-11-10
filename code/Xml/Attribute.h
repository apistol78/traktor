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
#if defined(T_XML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::xml
{

/*! XML Attribute.
 * \ingroup XML
 */
class T_DLLCLASS Attribute : public Object
{
	T_RTTI_CLASS;

public:
	Attribute(const std::wstring& name, const std::wstring& value = L"");

	virtual const std::wstring& getName() const;

	virtual void setName(const std::wstring& name);

	virtual const std::wstring& getValue() const;

	virtual void setValue(const std::wstring& value);

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
