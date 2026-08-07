/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vrfy/AccelerationStructureVrfy.h"

#include "Core/Misc/SafeDestroy.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.AccelerationStructureVrfy", AccelerationStructureVrfy, IAccelerationStructure)

AccelerationStructureVrfy::AccelerationStructureVrfy(IAccelerationStructure* wrappedAccelerationStructure)
	: m_wrappedAccelerationStructure(wrappedAccelerationStructure)
{
}

void AccelerationStructureVrfy::destroy()
{
	if (m_wrappedAccelerationStructure)
		m_wrappedAccelerationStructure->destroy();
}

}
