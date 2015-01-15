#ifndef traktor_amalgam_IEnvironment_H
#define traktor_amalgam_IEnvironment_H

#include "Core/Object.h"
#include "Amalgam/IAudioServer.h"
#include "Amalgam/IInputServer.h"
#include "Amalgam/IOnlineServer.h"
#include "Amalgam/IPhysicsServer.h"
#include "Amalgam/IRenderServer.h"
#include "Amalgam/IResourceServer.h"
#include "Amalgam/IScriptServer.h"
#include "Amalgam/IUpdateControl.h"
#include "Amalgam/IWorldServer.h"
#include "Amalgam/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace db
	{

class Database;

	}

	namespace amalgam
	{

/*! \brief Runtime environment.
 * \ingroup Amalgam
 *
 * This class represent the runtime environment.
 * It's a single accessors for applications to reach
 * various system objects.
 */
class T_DLLCLASS IEnvironment : public Object
{
	T_RTTI_CLASS;

public:
	virtual db::Database* getDatabase() = 0;

	virtual IUpdateControl* getControl() = 0;

	/*! \name Server accessors. */
	/*! \{ */

	virtual IAudioServer* getAudio() = 0;

	virtual IInputServer* getInput() = 0;

	virtual IOnlineServer* getOnline() = 0;

	virtual IPhysicsServer* getPhysics() = 0;

	virtual IRenderServer* getRender() = 0;

	virtual IResourceServer* getResource() = 0;

	virtual IScriptServer* getScript() = 0;

	virtual IWorldServer* getWorld() = 0;

	/*! \} */

	/*! \name Settings. */
	/*! \{ */

	virtual PropertyGroup* getSettings() = 0;

	virtual bool reconfigure() = 0;

	/*! \} */
};

	}
}

#endif	// traktor_amalgam_IEnvironment_H
