/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/Any.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Spark/Event.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Event", Event, Object)

int32_t Event::add(IRuntimeDelegate* rd)
{
	m_rds.push_back(rd);
	return (int32_t)(m_rds.size() - 1);
}

void Event::remove(int32_t index)
{
	T_ASSERT(index >= 0);
	T_ASSERT(index < (int32_t)m_rds.size());
	m_rds.erase(m_rds.begin() + index);
}

void Event::removeAll()
{
	m_rds.clear();
}

bool Event::empty() const
{
	return m_rds.empty();
}

void Event::issue()
{
	for (auto rd : m_rds)
		rd->call(0, nullptr);
}

void Event::issue(int32_t argc, const Any* argv)
{
	for (auto rd : m_rds)
		rd->call(argc, argv);
}

	}
}
