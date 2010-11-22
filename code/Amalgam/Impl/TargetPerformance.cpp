#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Amalgam/Impl/TargetPerformance.h"

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
,	memInUse(0)
,	heapObjects(0)
{
}

bool TargetPerformance::serialize(ISerializer& s)
{
	s >> Member< float >(L"time", time);
	s >> Member< float >(L"fps", fps);
	s >> Member< float >(L"update", update);
	s >> Member< float >(L"build", build);
	s >> Member< float >(L"render", render);
	s >> Member< float >(L"physics", physics);
	s >> Member< float >(L"input", input);
	s >> Member< uint32_t >(L"memAvail", memInUse);
	s >> Member< uint32_t >(L"heapObjects", heapObjects);
	return true;
}

	}
}
