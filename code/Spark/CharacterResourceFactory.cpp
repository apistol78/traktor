#include "Database/Database.h"
#include "Database/Instance.h"
#include "Spark/Character.h"
#include "Spark/CharacterResourceFactory.h"
#include "Spark/ICharacterResource.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.CharacterResourceFactory", CharacterResourceFactory, resource::IResourceFactory)

CharacterResourceFactory::CharacterResourceFactory(db::Database* db, render::IRenderSystem* renderSystem)
:	m_db(db)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet CharacterResourceFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	type_of< ICharacterResource >().findAllOf(typeSet);
	return typeSet;

}

const TypeInfoSet CharacterResourceFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	type_of< Character >().findAllOf(typeSet);
	return typeSet;
}

bool CharacterResourceFactory::isCacheable() const
{
	return true;
}

Ref< Object > CharacterResourceFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< ICharacterResource > resource = instance->getObject< ICharacterResource >();
	if (!resource)
		return 0;

	return resource->create(resourceManager, m_renderSystem, instance);
}

	}
}
