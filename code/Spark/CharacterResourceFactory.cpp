#include "Database/Database.h"
#include "Spark/Character.h"
#include "Spark/CharacterResourceFactory.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.CharacterResourceFactory", CharacterResourceFactory, resource::IResourceFactory)

CharacterResourceFactory::CharacterResourceFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet CharacterResourceFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	type_of< Character >().findAllOf(typeSet);
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
	return m_db->getObjectReadOnly< Character >(guid);
}

	}
}
