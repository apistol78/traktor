#ifndef traktor_amalgam_IOnlineServer_H
#define traktor_amalgam_IOnlineServer_H

#include "Amalgam/Game/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class ISessionManager;

	}

	namespace amalgam
	{

/*! \brief Online server.
 * \ingroup Amalgam
 */
class T_DLLCLASS IOnlineServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual online::ISessionManager* getSessionManager() = 0;
};

	}
}

#endif	// traktor_amalgam_IOnlineServer_H
