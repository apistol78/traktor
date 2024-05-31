/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Database/Remote/IMessage.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

/*! Guid result.
 * \ingroup Database
 */
class T_DLLCLASS MsgGuidResult : public IMessage
{
	T_RTTI_CLASS;

public:
	explicit MsgGuidResult(const Guid& value = Guid());

	const Guid& get() const { return m_value; }

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_value;
};

}
