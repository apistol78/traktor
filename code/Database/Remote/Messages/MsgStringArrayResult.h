/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Containers/AlignedVector.h"
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

/*! Array of string result.
 * \ingroup Database
 */
class T_DLLCLASS MsgStringArrayResult : public IMessage
{
	T_RTTI_CLASS;

public:
	MsgStringArrayResult() = default;

	explicit MsgStringArrayResult(const AlignedVector< std::wstring >& values);

	void add(const std::wstring& value);

	uint32_t count();

	const std::wstring& get(uint32_t index) const;

	const AlignedVector< std::wstring >& get() const { return m_values; }

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< std::wstring > m_values;
};

}
