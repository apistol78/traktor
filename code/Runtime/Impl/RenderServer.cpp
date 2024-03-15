/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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

double RenderServer::getCPUDuration() const
{
	return m_cpuDuration;
}

double RenderServer::getGPUDuration() const
{
	return m_gpuDuration;
}

int32_t RenderServer::getFrameRate() const
{
	return m_frameRate;
}

void RenderServer::setDurations(double cpuDuration, double gpuDuration)
{
	m_cpuDuration = cpuDuration;
	m_gpuDuration = gpuDuration;
}

void RenderServer::setFrameRate(int32_t frameRate)
{
	m_frameRate = frameRate;
}

int32_t RenderServer::getThreadFrameQueueCount() const
{
	return 2;
}

}
