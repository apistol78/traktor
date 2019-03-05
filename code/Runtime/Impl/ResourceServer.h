#pragma once

#include "Runtime/IResourceServer.h"

namespace traktor
{

class PropertyGroup;

	namespace db
	{

class Database;

	}

	namespace runtime
	{

class IEnvironment;

/*! \brief
 * \ingroup Runtime
 */
class ResourceServer : public IResourceServer
{
	T_RTTI_CLASS;

public:
	bool create(const PropertyGroup* settings, db::Database* database);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	void performCleanup();

	virtual resource::IResourceManager* getResourceManager() override final;

private:
	Ref< resource::IResourceManager > m_resourceManager;
};

	}
}

