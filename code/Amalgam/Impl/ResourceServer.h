#ifndef traktor_amalgam_ResourceServer_H
#define traktor_amalgam_ResourceServer_H

#include "Amalgam/IResourceServer.h"

namespace traktor
{

class PropertyGroup;

	namespace amalgam
	{

class IEnvironment;

class ResourceServer : public IResourceServer
{
	T_RTTI_CLASS;

public:
	bool create();

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);
	
	void dumpStatistics();

	virtual resource::IResourceManager* getResourceManager();

private:
	Ref< resource::IResourceManager > m_resourceManager;
};

	}
}

#endif	// traktor_amalgam_ResourceServer_H
