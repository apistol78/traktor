/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::runtime
{

class IStateManager;
class UpdateInfo;

/*! Runtime state.
 * \ingroup Runtime
 *
 * User applications implement multiple
 * classes from IState which represent
 * various states which the application can
 * be in such as FrontEnd, Loading, InGame etc.
 * Runtime is responsible of managing the
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

	/*! Enter state. */
	virtual void enter() = 0;

	/*! Leave state. */
	virtual void leave() = 0;

	/*! Update state.
	 *
	 * \param stateManager State manager.
	 * \param info Update information.
	 * \return Update result.
	 */
	virtual UpdateResult update(IStateManager* stateManager, const UpdateInfo& info) = 0;

	/*! Build frame.
	 *
	 * \param frame Build frame.
	 * \param info Update information.
	 * \return Render result.
	 */
	virtual BuildResult build(uint32_t frame, const UpdateInfo& info) = 0;

	/*! Render state.
	 *
	 * \param frame Render frame.
	 * \param info Update information.
	 * \return True if rendered successfully.
	 */
	virtual bool render(uint32_t frame, const UpdateInfo& info) = 0;

	/*! Take event.
	 *
	 * \param event Event.
	 * \return True if event handled.
	 */
	virtual bool take(const Object* event) = 0;
};

}
