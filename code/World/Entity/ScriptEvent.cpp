/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/ScriptEvent.h"

#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "World/Entity.h"
#include "World/Entity/ScriptComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ScriptEvent", ScriptEvent, IEntityEvent)

ScriptEvent::ScriptEvent(const std::wstring& method)
:	m_method(method)
,	m_methodName(wstombs(method))
{
}

Ref< IEntityEventInstance > ScriptEvent::createInstance(EventManagerComponent* eventManager, Entity* sender, const Transform& Toffset) const
{
	if (sender == nullptr)
	{
		log::warning << L"Unable to call script method \"" << m_method << L"\"; event has no sender." << Endl;
		return nullptr;
	}

	ScriptComponent* scriptComponent = sender->getComponent< ScriptComponent >();
	if (scriptComponent == nullptr)
	{
		log::warning << L"Unable to call script method \"" << m_method << L"\"; no script component in entity \"" << sender->getName() << L"\"." << Endl;
		return nullptr;
	}

	scriptComponent->execute(m_methodName.c_str());

	// Method has been called; no instance to keep alive.
	return nullptr;
}

}
