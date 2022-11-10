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
#include "Database/Events/EvtInstanceGuidChanged.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.EvtInstanceGuidChanged", 0, EvtInstanceGuidChanged, EvtInstance)

EvtInstanceGuidChanged::EvtInstanceGuidChanged(const Guid& instanceGuid, const Guid& instancePreviousGuid)
:	EvtInstance(instanceGuid)
,	m_instancePreviousGuid(instancePreviousGuid)
{
}

const Guid& EvtInstanceGuidChanged::getInstancePreviousGuid() const
{
	return m_instancePreviousGuid;
}

void EvtInstanceGuidChanged::serialize(ISerializer& s)
{
	EvtInstance::serialize(s);
	s >> Member< Guid >(L"instancePreviousGuid", m_instancePreviousGuid);
}

	}
}
