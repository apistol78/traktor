/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_ResourceServer_H
#define traktor_amalgam_ResourceServer_H

#include "Amalgam/Run/IResourceServer.h"
#include "Core/Ref.h"

namespace traktor
{

class PropertyGroup;

	namespace db
	{
	
class Database;

	}

	namespace amalgam
	{

class IEnvironment;

/*! \brief
 * \ingroup Amalgam
 */
class ResourceServer : public IResourceServer
{
	T_RTTI_CLASS;

public:
	bool create(const PropertyGroup* settings, db::Database* database);

	void destroy();

	virtual resource::IResourceManager* getResourceManager() T_OVERRIDE T_FINAL;

private:
	Ref< resource::IResourceManager > m_resourceManager;
};

	}
}

#endif	// traktor_amalgam_ResourceServer_H
