/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <string>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::net
{

/*! HTTP request.
 * \ingroup Net
 */
class T_DLLCLASS HttpRequest : public Object
{
	T_RTTI_CLASS;

public:
	enum Method
	{
		MtUnknown,
		MtGet,
		MtHead,
		MtPost,
		MtPut,
		MtDelete,
		MtTrace,
		MtOptions,
		MtConnect,
		MtPatch
	};

	Method getMethod() const;

	const std::wstring& getResource() const;

	bool hasValue(const std::wstring& key) const;

	void setValue(const std::wstring& key, const std::wstring& value);

	std::wstring getValue(const std::wstring& key) const;

	static Ref< HttpRequest > parse(const std::wstring& request);

private:
	Method m_method = MtUnknown;
	std::wstring m_resource;
	std::map< std::wstring, std::wstring > m_values;
};

}
