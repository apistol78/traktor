/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Model.h"
#include "Model/Operations/Clear.h"

namespace traktor::model
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Clear", Clear, IModelOperation)

Clear::Clear(uint32_t clearFlags)
:	m_clearFlags(clearFlags)
{
}

bool Clear::apply(Model& model) const
{
	model.clear(m_clearFlags);
	return true;
}

}
