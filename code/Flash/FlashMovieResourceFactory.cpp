#include "Database/Database.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieResourceFactory.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMovieResourceFactory", FlashMovieResourceFactory, resource::IResourceFactory)

FlashMovieResourceFactory::FlashMovieResourceFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet FlashMovieResourceFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< FlashMovie >());
	return typeSet;
}

const TypeInfoSet FlashMovieResourceFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< FlashMovie >());
	return typeSet;
}


bool FlashMovieResourceFactory::isCacheable() const
{
	return true;
}

Ref< Object > FlashMovieResourceFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	return m_db->getObjectReadOnly< FlashMovie >(guid);
}

	}
}
