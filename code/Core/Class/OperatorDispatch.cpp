/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/Any.h"
#include "Core/Class/OperatorDispatch.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.OperatorDispatch", OperatorDispatch, IRuntimeDispatch)

void OperatorDispatch::add(const IRuntimeDispatch* dispatch)
{
	m_dispatches.push_back(dispatch);
}

#if defined(T_NEED_RUNTIME_SIGNATURE)
void OperatorDispatch::signature(OutputStream& os) const {}
#endif

Any OperatorDispatch::invoke(ITypedObject* self, uint32_t argc, const Any* argv) const
{
	Any result;
	for (auto dispatch : m_dispatches)
	{
		result = dispatch->invoke(self, argc, argv);
		if (!result.isVoid())
			break;
	}
	return result;
}

}
