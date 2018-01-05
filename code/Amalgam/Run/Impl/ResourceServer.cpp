/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Run/IEnvironment.h"
#include "Amalgam/Run/Impl/ResourceServer.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Resource/ResourceManager.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ResourceServer", ResourceServer, IResourceServer)

bool ResourceServer::create(const PropertyGroup* settings, db::Database* database)
{
	m_resourceManager = new resource::ResourceManager(database, settings->getProperty< bool >(L"Resource.Verbose", false));
	return true;
}

void ResourceServer::destroy()
{
	safeDestroy(m_resourceManager);
}

resource::IResourceManager* ResourceServer::getResourceManager()
{
	return m_resourceManager;
}

	}
}
