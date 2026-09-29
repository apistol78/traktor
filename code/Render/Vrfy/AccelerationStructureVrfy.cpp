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
#include "Render/Vrfy/Error.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.AccelerationStructureVrfy", AccelerationStructureVrfy, IAccelerationStructure)

AccelerationStructureVrfy::AccelerationStructureVrfy(IAccelerationStructure* wrappedAccelerationStructure, bool pooled)
	: m_wrappedAccelerationStructure(wrappedAccelerationStructure)
	, m_pooled(pooled)
{
}

void AccelerationStructureVrfy::destroy()
{
	if (m_wrappedAccelerationStructure)
		m_wrappedAccelerationStructure->destroy();
}

void AccelerationStructureVrfy::nextFrame()
{
	T_CAPTURE_ASSERT(m_pooled, L"Acceleration structure not pooled.");
	m_pendingFrames++;
	if (m_wrappedAccelerationStructure)
		m_wrappedAccelerationStructure->nextFrame();
}

bool AccelerationStructureVrfy::beginWrite()
{
	if (!m_pooled)
		return true;
	if (m_pendingFrames == 0)
		return false;
	m_pendingFrames--;
	return true;
}

}
