/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Runtime/Target/TargetPerformance.h"

using namespace traktor;

namespace traktor::runtime
{
	namespace
	{

class MemberRenderSystemStatistics : public MemberComplex
{
public:
	MemberRenderSystemStatistics(const wchar_t* const name, render::RenderSystemStatistics& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< uint64_t >(L"memoryAvailable", m_ref.memoryAvailable);
		s >> Member< uint64_t >(L"memoryUsage", m_ref.memoryUsage);
		s >> Member< uint32_t >(L"buffers", m_ref.buffers);
		s >> Member< uint32_t >(L"simpleTextures", m_ref.simpleTextures);
		s >> Member< uint32_t >(L"cubeTextures", m_ref.cubeTextures);
		s >> Member< uint32_t >(L"volumeTextures", m_ref.volumeTextures);
		s >> Member< uint32_t >(L"renderTargetSets", m_ref.renderTargetSets);
		s >> Member< uint32_t >(L"programs", m_ref.programs);
	}

private:
	render::RenderSystemStatistics& m_ref;
};

class MemberRenderViewStatistics : public MemberComplex
{
public:
	MemberRenderViewStatistics(const wchar_t* const name, render::RenderViewStatistics& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< uint32_t >(L"passCount", m_ref.passCount);
		s >> Member< uint32_t >(L"drawCalls", m_ref.drawCalls);
		s >> Member< uint32_t >(L"primitiveCount", m_ref.primitiveCount);
	}

private:
	render::RenderViewStatistics& m_ref;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetPerfSet", TargetPerfSet, ISerializable)

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TpsRuntime", 0, TpsRuntime, TargetPerfSet)

bool TpsRuntime::check(const TargetPerfSet& old) const
{
	const TpsRuntime& o = (const TpsRuntime&)old;
	return	
		std::abs(fps - o.fps) >= 0.2f ||
		std::abs(update - o.update) >= 0.0001f ||
		std::abs(build - o.build) >= 0.0001f ||
		std::abs(renderCPU - o.renderCPU) >= 0.0001f ||
		std::abs(renderGPU - o.renderGPU) >= 0.0001f ||
		std::abs(physics - o.physics) >= 0.0001f ||
		std::abs(input - o.input) >= 0.0001f ||
		std::abs(garbageCollect - o.garbageCollect) >= 0.001f ||
		steps != o.steps ||
		std::abs(simulationInterval - o.simulationInterval) >= 0.01f ||
		std::abs(renderInterval - o.renderInterval) >= 0.01f ||
		collisions != o.collisions;
}

void TpsRuntime::serialize(ISerializer& s)
{
	s >> Member< float >(L"fps", fps);
	s >> Member< float >(L"update", update);
	s >> Member< float >(L"build", build);
	s >> Member< float >(L"renderCPU", renderCPU);
	s >> Member< float >(L"renderGPU", renderGPU);
	s >> Member< float >(L"physics", physics);
	s >> Member< float >(L"input", input);
	s >> Member< float >(L"garbageCollect", garbageCollect);
	s >> Member< int32_t >(L"steps", steps);
	s >> Member< float >(L"simulationInterval", simulationInterval);
	s >> Member< float >(L"renderInterval", renderInterval);
	s >> Member< uint32_t >(L"collisions", collisions);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TpsMemory", 0, TpsMemory, TargetPerfSet)

bool TpsMemory::check(const TargetPerfSet& old) const
{
	return false;
}

void TpsMemory::serialize(ISerializer& s)
{
	s >> Member< uint64_t >(L"memInUse", memInUse);
	s >> Member< uint32_t >(L"memInUseScript", memInUseScript);
	s >> Member< uint32_t >(L"heapObjects", heapObjects);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TpsRender", 0, TpsRender, TargetPerfSet)

bool TpsRender::check(const TargetPerfSet& old) const
{
	return false;
}

void TpsRender::serialize(ISerializer& s)
{
	s >> MemberRenderSystemStatistics(L"renderSystemStats", renderSystemStats);
	s >> MemberRenderViewStatistics(L"renderViewStats", renderViewStats);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TpsResource", 0, TpsResource, TargetPerfSet)

bool TpsResource::check(const TargetPerfSet& old) const
{
	const TpsResource& o = (const TpsResource&)old;
	return
		residentResourcesCount != o.residentResourcesCount ||
		exclusiveResourcesCount != o.exclusiveResourcesCount;
}

void TpsResource::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"residentResourcesCount", residentResourcesCount);
	s >> Member< uint32_t >(L"exclusiveResourcesCount", exclusiveResourcesCount);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TpsPhysics", 0, TpsPhysics, TargetPerfSet)

bool TpsPhysics::check(const TargetPerfSet& old) const
{
	const TpsPhysics& o = (const TpsPhysics&)old;
	return
		bodyCount != o.bodyCount ||
		activeBodyCount != o.activeBodyCount ||
		manifoldCount != o.manifoldCount ||
		queryCount != o.queryCount;
}

void TpsPhysics::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"bodyCount", bodyCount);
	s >> Member< uint32_t >(L"activeBodyCount", activeBodyCount);
	s >> Member< uint32_t >(L"manifoldCount", manifoldCount);
	s >> Member< uint32_t >(L"queryCount", queryCount);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TpsAudio", 0, TpsAudio, TargetPerfSet)

bool TpsAudio::check(const TargetPerfSet& old) const
{
	const TpsAudio& o = (const TpsAudio&)old;
	return
		activeSoundChannels != o.activeSoundChannels;
}

void TpsAudio::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"activeSoundChannels", activeSoundChannels);
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetPerformance", TargetPerformance, Object)

void TargetPerformance::publish(net::BidirectionalObjectTransport* transport, const TargetPerfSet& performance)
{
	const TypeInfo& performanceType = type_of(&performance);
	const double time = m_timer.getElapsedTime();

	// Check how long since last this performance set has been sent.
	const auto it = m_last.find(&performanceType);
	if (it != m_last.end())
	{
		if ((time - it->second.sent) < 1.0 / 10.0)
			return;
		if ((time - it->second.sent) < 4.0 && performance.check(*it->second.perfSet) == false)
			return;
	}

	if (!transport->send(&performance))
		return;

	// Save time and copy of performance set as a reference.
	auto& snapshot = m_last[&performanceType];
	snapshot.sent = time;
	snapshot.perfSet = DeepClone(&performance).create< TargetPerfSet >();

	// Ensure publishing doesn't take too long.
	const double duration = m_timer.getElapsedTime() - time;
	if (duration > 0.001)
		log::warning << L"Target performance publish exceed 1 ms." << Endl;
}

}
