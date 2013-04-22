#include "Database/Database.h"
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

VideoFactory::VideoFactory(db::Database* database, render::IRenderSystem* renderSystem)
:	m_database(database)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet VideoFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< VideoResource >());
	return typeSet;
}

const TypeInfoSet VideoFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Video >());
	return typeSet;
}

bool VideoFactory::isCacheable() const
{
	return true;
}

Ref< Object > VideoFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const
{
	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
		return 0;

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
