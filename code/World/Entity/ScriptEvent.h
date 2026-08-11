/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityEvent.h"

#include <string>

namespace traktor::world
{

/*! Script event.
 *
 * Call a method, without arguments, on the script component of the
 * entity issuing the event. As the method is called on the sender's
 * own script component it has access to the sender through "self".
 *
 * \ingroup World
 */
class ScriptEvent : public IEntityEvent
{
	T_RTTI_CLASS;

public:
	explicit ScriptEvent(const std::wstring& method);

	virtual Ref< IEntityEventInstance > createInstance(EventManagerComponent* eventManager, Entity* sender, const Transform& Toffset) const override final;

private:
	std::wstring m_method;
	std::string m_methodName;
};

}
