#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Runtime/Target/TargetPerformance.h"

using namespace traktor;

namespace traktor
{
	namespace runtime
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
		s >> Member< uint32_t >(L"vertexBuffers", m_ref.vertexBuffers);
		s >> Member< uint32_t >(L"indexBuffers", m_ref.indexBuffers);
		s >> Member< uint32_t >(L"structBuffers", m_ref.structBuffers);
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
		std::abs(fps - o.fps) >= 0.0001f ||
		std::abs(update - o.update) >= 0.0001f ||
		std::abs(build - o.build) >= 0.0001f ||
		std::abs(render - o.render) >= 0.0001f ||
		std::abs(physics - o.physics) >= 0.0001f ||
		std::abs(input - o.input) >= 0.0001f ||
		std::abs(garbageCollect - o.garbageCollect) >= 0.0001f ||
		std::abs(steps - o.steps) >= 0.0001f ||
		std::abs(interval - o.interval) >= 0.0001f ||
		collisions != o.collisions;
}

void TpsRuntime::serialize(ISerializer& s)
{
	s >> Member< float >(L"fps", fps);
	s >> Member< float >(L"update", update);
	s >> Member< float >(L"build", build);
	s >> Member< float >(L"render", render);
	s >> Member< float >(L"physics", physics);
	s >> Member< float >(L"input", input);
	s >> Member< float >(L"garbageCollect", garbageCollect);
	s >> Member< float >(L"steps", steps);
	s >> Member< float >(L"interval", interval);
	s >> Member< uint32_t >(L"collisions", collisions);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TpsMemory", 0, TpsMemory, TargetPerfSet)

bool TpsMemory::check(const TargetPerfSet& old) const
{
	const TpsMemory& o = (const TpsMemory&)old;
	return
		memInUse != o.memInUse ||
		memInUseScript != o.memInUseScript ||
		memCount != o.memCount ||
		heapObjects != o.heapObjects;
}

void TpsMemory::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"memInUse", memInUse);
	s >> Member< uint32_t >(L"memInUseScript", memInUseScript);
	s >> Member< int32_t >(L"memCount", memCount);
	s >> Member< uint32_t >(L"heapObjects", heapObjects);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TpsRender", 0, TpsRender, TargetPerfSet)

bool TpsRender::check(const TargetPerfSet& old) const
{
	const TpsRender& o = (const TpsRender&)old;
	return
		std::memcmp(&renderSystemStats, &o.renderSystemStats, sizeof(renderSystemStats)) != 0 ||
		std::memcmp(&renderViewStats, &o.renderViewStats, sizeof(renderViewStats)) != 0;
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

	auto it = m_last.find(&performanceType);
	if (it != m_last.end() && performance.check(*it->second) == false)
		return;

	if (!transport->send(&performance))
		return;

	m_last[&performanceType] = DeepClone(&performance).create< TargetPerfSet >();
}

	}
}
