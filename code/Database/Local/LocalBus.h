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
#include "Core/Guid.h"

namespace traktor
{

class ISharedMemory;

}

namespace traktor::db
{

/*! Local database event bus.
 * \ingroup Database
 *
 * Using named mutex to listen for new events across multiple
 * processes on the same machine.
 */
class LocalBus : public IProviderBus
{
	T_RTTI_CLASS;

public:
	explicit LocalBus(const std::wstring& journalFileName);

	virtual ~LocalBus();

	void close();

	virtual bool putEvent(const IEvent* event) override final;

	virtual bool getEvent(uint64_t& inoutSqnr, Ref< const IEvent >& outEvent, bool& outRemote) override final;

private:
	Guid m_localGuid;
	std::wstring m_journalFileName;
	Ref< ISharedMemory > m_shm;
};

}
