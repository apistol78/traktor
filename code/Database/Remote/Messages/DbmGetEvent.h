/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

/*! Get event from bus.
 * \ingroup Database
 */
class T_DLLCLASS DbmGetEvent : public IMessage
{
	T_RTTI_CLASS;

public:
	explicit DbmGetEvent(uint32_t handle = 0, uint64_t sqnr = 0);

	uint32_t getHandle() const { return m_handle; }

	uint64_t getSequenceNumber() const { return m_sqnr; }

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_handle;
	uint64_t m_sqnr;
};

}
