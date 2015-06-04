#ifndef traktor_amalgam_IResourceServer_H
#define traktor_amalgam_IResourceServer_H

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
	namespace resource
	{

class IResourceManager;

	}

	namespace amalgam
	{

/*! \brief Resource server.
 * \ingroup Amalgam
 */
class T_DLLCLASS IResourceServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual resource::IResourceManager* getResourceManager() = 0;
};

	}
}

#endif	// traktor_amalgam_IResourceServer_H
