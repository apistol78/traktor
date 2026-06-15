/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/PolymorphicDispatch.h"

#include "Core/Class/Any.h"
#include "Core/Io/OutputStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.PolymorphicDispatch", PolymorphicDispatch, IRuntimeDispatch)

void PolymorphicDispatch::set(uint32_t argc, const IRuntimeDispatch* dispatch)
{
	if (m_dispatches.size() < argc + 1)
		m_dispatches.resize(argc + 1);
	m_dispatches[argc].push_back(dispatch);
}

#if defined(T_NEED_RUNTIME_SIGNATURE)
void PolymorphicDispatch::signature(OutputStream& os) const
{
	bool first = true;
	for (const auto& dispatches : m_dispatches)
	{
		for (auto dispatch : dispatches)
		{
			if (dispatch)
			{
				if (!first)
					os << L";";
				dispatch->signature(os);
				first = false;
			}
		}
	}
}
#endif

Any PolymorphicDispatch::invoke(ITypedObject* self, uint32_t argc, const Any* argv) const
{
	if (argc >= m_dispatches.size()) [[unlikely]]
		return Any();

	const RefArray< const IRuntimeDispatch >& dispatches = m_dispatches[argc];

	// Common case; a single dispatch registered for given number of arguments.
	if (dispatches.size() == 1) [[likely]]
		return dispatches.front()->invoke(self, argc, argv);

	// Multiple dispatches share the same number of arguments; select the
	// first one which accept the given argument types.
	for (auto dispatch : dispatches)
		if (dispatch->accept(argc, argv))
			return dispatch->invoke(self, argc, argv);

	// No dispatch accepted the argument types; fall back to the first
	// registered so it can report a meaningful argument error.
	if (!dispatches.empty())
		return dispatches.front()->invoke(self, argc, argv);

	return Any();
}

}
