//#include "Video/Video.h"
#include "Video/VideoFactory.h"

namespace traktor
{
	namespace video
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.video.VideoFactory", VideoFactory, resource::IResourceFactory)

VideoFactory::VideoFactory(db::Database* database)
:	m_database(database)
{
}

const TypeInfoSet VideoFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	//typeSet.insert(&type_of< Video >());
	return typeSet;
}

bool VideoFactory::isCacheable() const
{
	return true;
}

Ref< Object > VideoFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	return 0;
}

	}
}
