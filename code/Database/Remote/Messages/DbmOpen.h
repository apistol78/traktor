/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Database/Remote/IMessage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

/*! Open database.
 * \ingroup Database
 */
class T_DLLCLASS DbmOpen : public IMessage
{
	T_RTTI_CLASS;

public:
	explicit DbmOpen(const std::wstring_view& name = L"");

	const std::wstring& getName() const { return m_name; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
};

}
