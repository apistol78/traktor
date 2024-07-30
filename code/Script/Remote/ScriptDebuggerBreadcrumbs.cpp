/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Script/Remote/ScriptDebuggerBreadcrumbs.h"

namespace traktor::script
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptDebuggerBreadcrumbs", 0, ScriptDebuggerBreadcrumbs, ISerializable)

ScriptDebuggerBreadcrumbs::ScriptDebuggerBreadcrumbs(const AlignedVector< uint32_t >& breadcrumbs)
:	m_breadcrumbs(breadcrumbs)
{
}

void ScriptDebuggerBreadcrumbs::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< uint32_t >(L"breadcrumbs", m_breadcrumbs);
}

}
