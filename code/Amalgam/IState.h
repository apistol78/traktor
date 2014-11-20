#ifndef traktor_amalgam_IState_H
#define traktor_amalgam_IState_H

#include "Core/Object.h"
#include "Render/Types.h"

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

class IEvent;
class IStateManager;
class IUpdateControl;
class IUpdateInfo;

/*! \brief Runtime state.
 * \ingroup Amalgam
 *
 * User applications implement multiple
 * classes from IState which represent
 * various states which the application can
 * be in such as FrontEnd, Loading, InGame etc.
 * Amalgam is responsible of managing the
 * transition safely between different states
 * by, for instance, synchronizing the renderer etc.
 */
class T_DLLCLASS IState : public Object
{
	T_RTTI_CLASS;

public:
	enum UpdateResult
	{
		UrFailed = -1,	//< Update failed somehow, enter error recovery state.
		UrOk = 0,		//< Update succeeded.
		UrExit = 2		//< Update succeeded but wants to terminate application.
	};

	enum BuildResult
	{
		BrFailed = -1,	//< Build failed somehow, enter error recovery state.
		BrOk = 0,		//< Build succeeded,
		BrNothing = 1	//< Nothing built, fall back on default renderer.
	};

	/*! \brief Enter state. */
	virtual void enter() = 0;

	/*! \brief Leave state. */
	virtual void leave() = 0;

	/*! \brief Update state.
	 *
	 * \param stateManager State manager.
	 * \param control Update control.
	 * \param info Update information.
	 * \return Update result.
	 */
	virtual UpdateResult update(IStateManager* stateManager, IUpdateControl& control, const IUpdateInfo& info) = 0;

	/*! \brief Build frame.
	 *
	 * \param frame Build frame.
	 * \param info Update information.
	 * \return Render result.
	 */
	virtual BuildResult build(uint32_t frame, const IUpdateInfo& info) = 0;

	/*! \brief Render state.
	 *
	 * \param frame Render frame.
	 * \param eye Render eye.
	 * \param info Update information.
	 * \return True if rendered successfully.
	 */
	virtual bool render(uint32_t frame, render::EyeType eye, const IUpdateInfo& info) = 0;

	/*! \brief Flush built frames.
	 */
	virtual void flush() = 0;

	/*! \brief Take event.
	 *
	 * \param event Event.
	 * \return True if event handled.
	 */
	virtual bool take(const IEvent* event) = 0;
};

	}
}

#endif	// traktor_amalgam_IState_H
