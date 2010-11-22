#ifndef traktor_amalgam_ErrorState_H
#define traktor_amalgam_ErrorState_H

#include "Amalgam/IState.h"

namespace traktor
{
	namespace amalgam
	{

class Environment;

class ErrorState : public IState
{
	T_RTTI_CLASS;

public:
	ErrorState(Environment* environment);

	virtual void enter();

	virtual void leave();

	virtual UpdateResult update(IStateManager* stateManager, IUpdateControl& control, const IUpdateInfo& info, bool substep);

	virtual BuildResult build(uint32_t frame, const IUpdateInfo& info);

	virtual bool render(uint32_t frame, render::EyeType eye, const IUpdateInfo& info);

private:
	Ref< Environment > m_environment;
};

	}
}

#endif	// traktor_amalgam_ErrorState_H
