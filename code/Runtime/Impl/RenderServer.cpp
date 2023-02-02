/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Impl/RenderServer.h"
#include "Core/Thread/Atomic.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.RenderServer", RenderServer, IRenderServer)

RenderServer::UpdateResult RenderServer::update(PropertyGroup* settings)
{
	return UrSuccess;
}

int32_t RenderServer::getFrameRate() const
{
	return m_frameRate;
}

void RenderServer::setFrameRate(int32_t frameRate)
{
	Atomic::exchange(m_frameRate, frameRate);
}

int32_t RenderServer::getThreadFrameQueueCount() const
{
	return 2;
}

}
