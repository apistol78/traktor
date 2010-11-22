#ifndef traktor_amalgam_TargetInstance_H
#define traktor_amalgam_TargetInstance_H

#include "Core/Object.h"
#include "Amalgam/Impl/TargetPerformance.h"

namespace traktor
{
	namespace amalgam
	{

class Target;

enum TargetState
{
	TsIdle,
	TsPending,
	TsBuilding,
	TsDeploying,
	TsLaunching,
	TsRunning
};

class TargetInstance : public Object
{
	T_RTTI_CLASS;

public:
	TargetInstance(const std::wstring& name, const Target* target);

	const std::wstring& getName() const;

	const Target* getTarget() const;

	void setState(TargetState state);

	TargetState getState() const;

	void setBuildProgress(int32_t buildProgress);

	int32_t getBuildProgress() const;

	void setPerformance(const TargetPerformance& performance);

	const TargetPerformance& getPerformance() const;

private:
	std::wstring m_name;
	Ref< const Target > m_target;
	TargetState m_state;
	int32_t m_buildProgress;
	TargetPerformance m_performance;
};

	}
}

#endif	// traktor_amalgam_TargetInstance_H
