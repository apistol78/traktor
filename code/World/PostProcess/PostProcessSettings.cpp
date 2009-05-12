#include "World/PostProcess/PostProcessSettings.h"
#include "World/PostProcess/PostProcessDefine.h"
#include "World/PostProcess/PostProcessStep.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.PostProcessSettings", PostProcessSettings, Serializable)

const RefArray< PostProcessDefine >& PostProcessSettings::getDefinitions() const
{
	return m_definitions;
}

const RefArray< PostProcessStep >& PostProcessSettings::getSteps() const
{
	return m_steps;
}

bool PostProcessSettings::serialize(Serializer& s)
{
	s >> MemberRefArray< PostProcessDefine >(L"definitions", m_definitions);
	s >> MemberRefArray< PostProcessStep >(L"steps", m_steps);
	return true;
}

	}
}
