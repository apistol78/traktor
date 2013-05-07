#ifndef traktor_amalgam_StageState_H
#define traktor_amalgam_StageState_H

#include "Amalgam/IState.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;

	}

	namespace amalgam
	{

class Stage;

class T_DLLCLASS StageState : public amalgam::IState
{
	T_RTTI_CLASS;

public:
	StageState(
		amalgam::IEnvironment* environment,
		Stage* stage
	);

	virtual void enter();

	virtual void leave();

	virtual UpdateResult update(amalgam::IStateManager* stateManager, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info);

	virtual BuildResult build(uint32_t frame, const amalgam::IUpdateInfo& info);

	virtual bool render(uint32_t frame, render::EyeType eye, const amalgam::IUpdateInfo& info);

	virtual bool take(const amalgam::IEvent* event);

private:
	Ref< amalgam::IEnvironment > m_environment;
	Ref< Stage > m_stage;
};

	}
}

#endif	// traktor_amalgam_StageState_H
