#include "Amalgam/TargetPerformance.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberAlignedVector.h"

using namespace traktor;

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.TargetPerformance", 0, TargetPerformance, ISerializable)

TargetPerformance::TargetPerformance()
:	time(0.0f)
,	fps(0.0f)
,	update(0.0f)
,	build(0.0f)
,	render(0.0f)
,	physics(0.0f)
,	input(0.0f)
,	garbageCollect(0.0f)
,	steps(0.0f)
,	interval(0.0f)
,	collisions(0)
,	memInUse(0)
,	memInUseScript(0)
,	heapObjects(0)
,	drawCalls(0)
,	primitiveCount(0)
,	residentResourcesCount(0)
,	exclusiveResourcesCount(0)
,	bodyCount(0)
,	activeBodyCount(0)
,	manifoldCount(0)
,	queryCount(0)
,	activeSoundChannels(0)
,	flashCharacterCount(0)
{
}

void TargetPerformance::serialize(ISerializer& s)
{
	s >> Member< float >(L"time", time);
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
	s >> Member< uint32_t >(L"memInUse", memInUse);
	s >> Member< uint32_t >(L"memInUseScript", memInUseScript);
	s >> Member< uint32_t >(L"heapObjects", heapObjects);
	s >> Member< uint32_t >(L"drawCalls", drawCalls);
	s >> Member< uint32_t >(L"primitiveCount", primitiveCount);
	s >> Member< uint32_t >(L"residentResourcesCount", residentResourcesCount);
	s >> Member< uint32_t >(L"exclusiveResourcesCount", exclusiveResourcesCount);
	s >> Member< uint32_t >(L"bodyCount", bodyCount);
	s >> Member< uint32_t >(L"activeBodyCount", activeBodyCount);
	s >> Member< uint32_t >(L"manifoldCount", manifoldCount);
	s >> Member< uint32_t >(L"queryCount", queryCount);
	s >> Member< uint32_t >(L"activeSoundChannels", activeSoundChannels);
	s >> Member< uint32_t >(L"flashCharacterCount", flashCharacterCount);
	s >> MemberAlignedVector< FrameMarker, MemberComposite< FrameMarker > >(L"frameMarkers", frameMarkers);
}

void TargetPerformance::FrameMarker::serialize(ISerializer& s)
{
	s >> Member< uint8_t >(L"id", id);
	s >> Member< uint8_t >(L"level", level);
	s >> Member< float >(L"begin", begin);
	s >> Member< float >(L"end", end);
}

	}
}
