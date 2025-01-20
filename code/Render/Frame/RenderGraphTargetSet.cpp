/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Frame/RenderGraphTargetSet.h"

#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderTargetSet.h"

namespace traktor::render
{

RenderGraphTargetSet::RenderGraphTargetSet(IRenderTargetSet* readTargetSet, IRenderTargetSet* writeTargetSet)
	: m_readTargetSet(readTargetSet)
	, m_writeTargetSet(writeTargetSet)
{
}

void RenderGraphTargetSet::destroy()
{
	if (m_readTargetSet == m_writeTargetSet)
	{
		safeDestroy(m_readTargetSet);
		m_writeTargetSet = nullptr;
	}
	else
	{
		safeDestroy(m_readTargetSet);
		safeDestroy(m_writeTargetSet);
	}
}

void RenderGraphTargetSet::swap()
{
	Ref< IRenderTargetSet > tmp = m_readTargetSet;
	m_readTargetSet = m_writeTargetSet;
	m_writeTargetSet = tmp;
}

}
