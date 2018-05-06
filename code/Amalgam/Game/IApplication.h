/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_IApplication_H
#define traktor_amalgam_IApplication_H

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
	#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;
class IStateManager;

/*! \brief Amalgam application.
 * \ingroup Amalgam
 *
 * This interface represent the running
 * instance of the application.
 */
class T_DLLCLASS IApplication : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Get runtime environment.
	 *
	 * \return Runtime environment.
	 */
	virtual Ref< IEnvironment > getEnvironment() = 0;

	/*! \brief Get state manager.
	 *
	 * \return State manager.
	 */
	virtual Ref< IStateManager > getStateManager() = 0;
};

	}
}

#endif	// traktor_amalgam_IApplication_H
