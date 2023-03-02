/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Database/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

class IEvent;

/*! Provider event bus interface.
 * \ingroup Database
 *
 * The bus is used to communicate changes made to other
 * peers which is connected to the same database.
 *
 * The high level database layer puts events when
 * for example an instance has been commits.
 */
class T_DLLCLASS IProviderBus : public Object
{
	T_RTTI_CLASS;

public:
	/*! Put an event onto the bus.
	 *
	 * \param event Event instance.
	 * \return True if event has been put onto the bus.
	 */
	virtual bool putEvent(const IEvent* event) = 0;

	/*! Get event from bus.
	 *
	 * \param inoutSqnr Sequence number of last retrieved event, will return sequence number of returned event.
	 * \param outEvent Event instance.
	 * \param outRemote True if event originates from another connection; ie. another process.
	 * \return True if event was read from bus, false if no events are available.
	 */
	virtual bool getEvent(uint64_t& inoutSqnr, Ref< const IEvent >& outEvent, bool& outRemote) = 0;
};

}
