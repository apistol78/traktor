#include "Ai/NavMeshFactory.h"
#include "Amalgam/IEnvironment.h"
#include "Amalgam/Impl/ResourceServer.h"
#include "Animation/Animation/AnimationFactory.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Flash/FlashMovieResourceFactory.h"
#include "Heightfield/HeightfieldFactory.h"
#include "Mesh/MeshFactory.h"
#include "Spray/EffectFactory.h"
#include "Video/VideoFactory.h"
#include "Weather/Clouds/CloudMaskFactory.h"
#include "Resource/ResourceManager.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ResourceServer", ResourceServer, IResourceServer)

bool ResourceServer::create(const PropertyGroup* settings)
{
	m_resourceManager = new resource::ResourceManager(settings->getProperty< PropertyBoolean >(L"Resource.Verbose", false));
	return true;
}

void ResourceServer::destroy()
{
	safeDestroy(m_resourceManager);
}

void ResourceServer::createResourceFactories(IEnvironment* environment)
{
	render::IRenderSystem* renderSystem = environment->getRender()->getRenderSystem();
	const world::IEntityBuilder* entityBuilder = environment->getWorld()->getEntityBuilder();
	db::Database* database = environment->getDatabase();

	m_resourceManager->addFactory(new ai::NavMeshFactory(database));
	m_resourceManager->addFactory(new animation::AnimationFactory(database));
	m_resourceManager->addFactory(new mesh::MeshFactory(database, renderSystem));
	m_resourceManager->addFactory(new spray::EffectFactory(database, entityBuilder));
	m_resourceManager->addFactory(new flash::FlashMovieResourceFactory(database));
	m_resourceManager->addFactory(new hf::HeightfieldFactory(database));
	m_resourceManager->addFactory(new video::VideoFactory(database, renderSystem));
	m_resourceManager->addFactory(new weather::CloudMaskFactory(database));
}

int32_t ResourceServer::reconfigure(const PropertyGroup* settings)
{
	return CrUnaffected;
}

void ResourceServer::performCleanup()
{
	m_resourceManager->unloadUnusedResident();
}

resource::IResourceManager* ResourceServer::getResourceManager()
{
	return m_resourceManager;
}

	}
}
