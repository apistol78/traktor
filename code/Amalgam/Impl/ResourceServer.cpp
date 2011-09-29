#include "Amalgam/IEnvironment.h"
#include "Amalgam/Impl/ResourceServer.h"
#include "Animation/Animation/AnimationFactory.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/FlashMovieResourceFactory.h"
#include "Heightfield/HeightfieldFactory.h"
#include "Heightfield/MaterialMaskFactory.h"
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

bool ResourceServer::create()
{
	m_resourceManager = new resource::ResourceManager();
	return true;
}

void ResourceServer::destroy()
{
	safeDestroy(m_resourceManager);
}

void ResourceServer::createResourceFactories(IEnvironment* environment)
{
	render::IRenderSystem* renderSystem = environment->getRender()->getRenderSystem();
	db::Database* database = environment->getDatabase();

	m_resourceManager->addFactory(new animation::AnimationFactory(database));
	m_resourceManager->addFactory(new mesh::MeshFactory(database, renderSystem));
	m_resourceManager->addFactory(new spray::EffectFactory(database));
	m_resourceManager->addFactory(new flash::FlashMovieResourceFactory(database));
	m_resourceManager->addFactory(new hf::HeightfieldFactory(database));
	m_resourceManager->addFactory(new hf::MaterialMaskFactory(database));
	m_resourceManager->addFactory(new video::VideoFactory(database, renderSystem));
	m_resourceManager->addFactory(new weather::CloudMaskFactory(database));
}

int32_t ResourceServer::reconfigure(const Settings* settings)
{
	return CrUnaffected;
}

void ResourceServer::dumpStatistics()
{
	m_resourceManager->dumpStatistics();
}

resource::IResourceManager* ResourceServer::getResourceManager()
{
	return m_resourceManager;
}

	}
}
