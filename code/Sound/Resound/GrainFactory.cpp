#include "Sound/Resound/GrainFactory.h"
#include "Sound/Resound/IGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GrainFactory", GrainFactory, IGrainFactory)

GrainFactory::GrainFactory(resource::IResourceManager* resourceManager)
:	m_resourceManager(resourceManager)
{
}

resource::IResourceManager* GrainFactory::getResourceManager()
{
	return m_resourceManager;
}

Ref< IGrain > GrainFactory::createInstance(const IGrainData* grainData)
{
	return grainData ? grainData->createInstance(this) : 0;
}

	}
}
