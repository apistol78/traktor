/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/IAccelerationStructure.h"

namespace traktor::render
{

class Context;

class AccelerationStructureVk : public IAccelerationStructure
{
	T_RTTI_CLASS;

public:
	explicit AccelerationStructureVk(Context* context);

protected:
	Context* m_context = nullptr;
};

}
