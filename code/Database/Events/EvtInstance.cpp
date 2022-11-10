/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Database/Events/EvtInstance.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.EvtInstance", EvtInstance, IEvent)

EvtInstance::EvtInstance(const Guid& instanceGuid)
:	m_instanceGuid(instanceGuid)
{
}

const Guid& EvtInstance::getInstanceGuid() const
{
	return m_instanceGuid;
}

void EvtInstance::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"instanceGuid", m_instanceGuid);
}

	}
}
