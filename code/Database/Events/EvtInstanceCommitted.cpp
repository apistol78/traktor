/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Events/EvtInstanceCommitted.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.EvtInstanceCommitted", 0, EvtInstanceCommitted, EvtInstance)

EvtInstanceCommitted::EvtInstanceCommitted(const Guid& instanceGuid)
:	EvtInstance(instanceGuid)
{
}

	}
}
