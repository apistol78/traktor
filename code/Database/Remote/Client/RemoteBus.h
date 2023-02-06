/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Database/Provider/IProviderBus.h"

namespace traktor::db
{

class RemoteConnection;

/*! Remote event bus.
 * \ingroup Database
 */
class RemoteBus : public IProviderBus
{
	T_RTTI_CLASS;

public:
	explicit RemoteBus(RemoteConnection* connection, uint32_t handle);

	virtual ~RemoteBus();

	virtual bool putEvent(const IEvent* event) override final;

	virtual bool getEvent(uint64_t& inoutSqnr, Ref< const IEvent >& outEvent, bool& outRemote) override final;

private:
	Ref< RemoteConnection > m_connection;
	uint32_t m_handle;
};

}
