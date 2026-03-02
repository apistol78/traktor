/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"

namespace traktor::render
{

/*!
 * \ingroup Vrfy
 */
class ResourceTracker : public Object
{
public:
	void add(const Object* resource);

	void remove(const Object* resource);

	void snapshot();

	void alive(const TypeInfo& resourceType, bool diff) const;

	uint32_t count(const TypeInfo& resourceType) const;

private:
	struct Data
	{
		void* callstack[32] = { nullptr };
		int32_t callstackDepth = 0;
	};

	mutable Semaphore m_lock;
	SmallMap< const Object*, Data > m_data;
	SmallMap< const Object*, Data > m_snapshot;
};

}
