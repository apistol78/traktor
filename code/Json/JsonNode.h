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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_JSON_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Any;
class OutputStream;

}

namespace traktor::json
{

/*! JSON abstract node.
 * \ingroup JSON
 */
class T_DLLCLASS JsonNode : public Object
{
	T_RTTI_CLASS;

public:
	/*! Write JSON respresentation of node into stream.
	 *
	 * \param os Output stream.
	 * \return True if successfully written.
	 */
	virtual bool write(OutputStream& os) const = 0;

protected:
	/*! Write a string as an RFC 8259 compliant, escaped JSON string literal.
	 *
	 * Emits the enclosing double quotes and escapes control characters and the
	 * reserved \" and \\ characters.
	 *
	 * \param os Output stream.
	 * \param str String to write.
	 */
	static void writeString(OutputStream& os, const std::wstring& str);

	/*! Write a scalar or node value as RFC 8259 compliant JSON.
	 *
	 * \param os Output stream.
	 * \param value Value to write.
	 */
	static void writeValue(OutputStream& os, const Any& value);
};

}
