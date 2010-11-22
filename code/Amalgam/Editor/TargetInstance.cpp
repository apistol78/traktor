#include "Amalgam/Editor/TargetInstance.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetInstance", TargetInstance, Object)

TargetInstance::TargetInstance(const std::wstring& name, const Target* target)
:	m_name(name)
,	m_target(target)
,	m_state(TsIdle)
{
}

const std::wstring& TargetInstance::getName() const
{
	return m_name;
}

const Target* TargetInstance::getTarget() const
{
	return m_target;
}

void TargetInstance::setState(TargetState state)
{
	m_state = state;
}

TargetState TargetInstance::getState() const
{
	return m_state;
}

void TargetInstance::setBuildProgress(int32_t buildProgress)
{
	m_buildProgress = buildProgress;
}

int32_t TargetInstance::getBuildProgress() const
{
	return m_buildProgress;
}

void TargetInstance::setPerformance(const TargetPerformance& performance)
{
	m_performance = performance;
}

const TargetPerformance& TargetInstance::getPerformance() const
{
	return m_performance;
}

	}
}
