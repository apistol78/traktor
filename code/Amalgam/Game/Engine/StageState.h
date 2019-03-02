#pragma once

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

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS StageState : public IState
{
	T_RTTI_CLASS;

public:
	StageState(
		IEnvironment* environment,
		Stage* stage
	);

	virtual void enter() override final;

	virtual void leave() override final;

	virtual UpdateResult update(IStateManager* stateManager, const UpdateInfo& info) override final;

	virtual BuildResult build(uint32_t frame, const UpdateInfo& info) override final;

	virtual bool render(uint32_t frame, render::EyeType eye, const UpdateInfo& info) override final;

	virtual void flush() override final;

	virtual bool take(const Object* event) override final;

private:
	Ref< IEnvironment > m_environment;
	Ref< Stage > m_stage;
};

	}
}

