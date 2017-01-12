#include "Database/Instance.h"
#include "Video/Video.h"
#include "Video/VideoFactory.h"
#include "Video/VideoResource.h"
#include "Video/Decoders/VideoDecoderTheora.h"

namespace traktor
{
	namespace video
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.video.VideoFactory", VideoFactory, resource::IResourceFactory)

VideoFactory::VideoFactory(render::IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

const TypeInfoSet VideoFactory::getResourceTypes() const
{
	return makeTypeInfoSet< VideoResource >();
}

const TypeInfoSet VideoFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Video >();
}

bool VideoFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > VideoFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	Ref< VideoDecoderTheora > decoder = new VideoDecoderTheora();
	if (!decoder->create(stream))
		return 0;

	Ref< Video > video = new Video();
	if (!video->create(m_renderSystem, decoder))
		return 0;

	return video;
}

	}
}
