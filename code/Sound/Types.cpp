/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <string>
#include "Core/Containers/SmallMap.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/CriticalSection.h"
#include "Sound/Types.h"

namespace traktor::sound
{
	namespace
	{

class HandleRegistry
{
public:
	HandleRegistry()
	:	m_nextUnusedHandle(1)
	{
	}

	handle_t getHandle(const std::wstring& name)
	{
		T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_lock);

		SmallMap< std::wstring, handle_t >::const_iterator i = m_handles.find(name);
		if (i != m_handles.end())
		{
			T_ASSERT(i->second > 0);
			return i->second;
		}

		const handle_t handle = m_nextUnusedHandle++;
		m_handles.insert(std::make_pair(name, handle));

		return handle;
	}

private:
	CriticalSection m_lock;
	SmallMap< std::wstring, handle_t > m_handles;
	handle_t m_nextUnusedHandle;
};

HandleRegistry s_handleRegistry;

	}

handle_t getParameterHandle(const std::wstring& name)
{
	return s_handleRegistry.getHandle(name);
}

}
