#ifndef traktor_parade_LoadState_H
#define traktor_parade_LoadState_H

#include "Amalgam/IState.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Job;

	namespace amalgam
	{

class IEnvironment;

	}

	namespace parade
	{

class Stage;

class T_DLLCLASS LoadState : public amalgam::IState
{
	T_RTTI_CLASS;

public:
	LoadState(
		amalgam::IEnvironment* environment,
		const Guid& stageGuid,
		const Object* stageParams
	);

	virtual void enter();

	virtual void leave();

	virtual UpdateResult update(amalgam::IStateManager* stateManager, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info);

	virtual BuildResult build(uint32_t frame, const amalgam::IUpdateInfo& info);

	virtual bool render(uint32_t frame, render::EyeType eye, const amalgam::IUpdateInfo& info);

	virtual bool take(const amalgam::IEvent* event);

private:
	Ref< amalgam::IEnvironment > m_environment;
	Guid m_stageGuid;
	Ref< const Object > m_stageParams;
	Ref< Job > m_jobLoader;
	Ref< amalgam::IState > m_nextState;

	void jobLoader();
};

	}
}

#endif	// traktor_parade_LoadState_H
