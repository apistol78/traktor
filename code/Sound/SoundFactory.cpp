#include "Database/Database.h"
#include "Database/Instance.h"
#include "Sound/ISoundResource.h"
#include "Sound/Sound.h"
#include "Sound/SoundFactory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundFactory", SoundFactory, resource::IResourceFactory)

SoundFactory::SoundFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet SoundFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	type_of< ISoundResource >().findAllOf(typeSet);
	return typeSet;
}

const TypeInfoSet SoundFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	type_of< Sound >().findAllOf(typeSet);
	return typeSet;
}

bool SoundFactory::isCacheable() const
{
	return true;
}

Ref< Object > SoundFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< ISoundResource > resource = instance->getObject< ISoundResource >();
	if (!resource)
		return 0;

	return resource->createSound(resourceManager, instance);
}

	}
}
