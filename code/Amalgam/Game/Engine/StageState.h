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

	virtual void enter() T_OVERRIDE T_FINAL;

	virtual void leave() T_OVERRIDE T_FINAL;

	virtual UpdateResult update(IStateManager* stateManager, const UpdateInfo& info) T_OVERRIDE T_FINAL;

	virtual BuildResult build(uint32_t frame, const UpdateInfo& info) T_OVERRIDE T_FINAL;

	virtual bool render(uint32_t frame, render::EyeType eye, const UpdateInfo& info) T_OVERRIDE T_FINAL;

	virtual void flush() T_OVERRIDE T_FINAL;

	virtual bool take(const Object* event) T_OVERRIDE T_FINAL;

private:
	Ref< IEnvironment > m_environment;
	Ref< Stage > m_stage;
};

	}
}

#endif	// traktor_amalgam_StageState_H
