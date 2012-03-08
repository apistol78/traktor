#ifndef traktor_amalgam_RenderServer_H
#define traktor_amalgam_RenderServer_H

#include "Amalgam/IRenderServer.h"

namespace traktor
{

class PropertyGroup;

	namespace amalgam
	{

class IEnvironment;

class RenderServer : public IRenderServer
{
	T_RTTI_CLASS;

public:
	enum UpdateResult
	{
		UrTerminate = 0,
		UrSuccess = 1,
		UrReconfigure = 2
	};

	virtual void destroy() = 0;

	virtual void createResourceFactories(IEnvironment* environment) = 0;

	virtual int32_t reconfigure(const PropertyGroup* settings) = 0;

	virtual UpdateResult update(PropertyGroup* settings) = 0;
};

	}
}

#endif	// traktor_amalgam_RenderServer_H
