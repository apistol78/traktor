#ifndef traktor_amalgam_StageState_H
#define traktor_amalgam_StageState_H

#include "Amalgam/Game/IState.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;
class Stage;

class T_DLLCLASS StageState : public IState
{
	T_RTTI_CLASS;

public:
	StageState(
		IEnvironment* environment,
		Stage* stage
	);

	virtual void enter();

	virtual void leave();

	virtual UpdateResult update(IStateManager* stateManager, const UpdateInfo& info);

	virtual BuildResult build(uint32_t frame, const UpdateInfo& info);

	virtual bool render(uint32_t frame, render::EyeType eye, const UpdateInfo& info);

	virtual void flush();

	virtual bool take(const IEvent* event);

private:
	Ref< IEnvironment > m_environment;
	Ref< Stage > m_stage;
};

	}
}

#endif	// traktor_amalgam_StageState_H
