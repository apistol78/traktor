/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Engine/EntityResult.h"

#include "World/Entity.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.EntityResult", EntityResult, Result)

void EntityResult::succeed(world::Entity* entity)
{
	m_entity = entity;
	Result::succeed();
}

world::Entity* EntityResult::get() const
{
	wait();
	return m_entity;
}

void EntityResult::cancel()
{
	m_cancelled = true;
}

bool EntityResult::cancelled() const
{
	return m_cancelled;
}

}
