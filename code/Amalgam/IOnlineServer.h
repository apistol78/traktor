#ifndef traktor_amalgam_IOnlineServer_H
#define traktor_amalgam_IOnlineServer_H

#include "Amalgam/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
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

/*! \brief Online server. */
class T_DLLCLASS IOnlineServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual online::ISessionManager* getSessionManager() = 0;
};

	}
}

#endif	// traktor_amalgam_IOnlineServer_H
