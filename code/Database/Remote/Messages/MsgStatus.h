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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

/*! Status codes.
 * \ingroup Database
 */
enum MsgStatusType
{
	StSuccess = 0,
	StFailure = -1,
	StNoReply = -2,
	StInvalidReply = -3
};

/*! Status result.
 * \ingroup Database
 */
class T_DLLCLASS MsgStatus : public IMessage
{
	T_RTTI_CLASS;

public:
	explicit MsgStatus(MsgStatusType status = StSuccess);

	MsgStatusType getStatus() const { return (MsgStatusType)m_status; }

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_status;
};

}
