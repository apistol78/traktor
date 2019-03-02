#pragma once

#include "Amalgam/Run/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_RUN_EXPORT)
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

