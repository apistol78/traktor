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
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

}

namespace traktor::net
{

/*!
 * \ingroup Net
 */
class T_DLLCLASS HttpResponse : public Object
{
	T_RTTI_CLASS;

public:
	bool parse(IStream* stream);

	int32_t getStatusCode() const;

	const std::wstring& getStatusMessage() const;

	void set(const std::wstring& name, const std::wstring& value);

	std::wstring get(const std::wstring& name) const;

private:
	std::map< std::wstring, std::wstring > m_values;
	std::wstring m_statusMessage;
	int32_t m_statusCode = 0;
};

}
