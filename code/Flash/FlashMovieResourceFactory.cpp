#include "Flash/FlashMovieResourceFactory.h"
#include "Flash/FlashMovieResource.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/SwfReader.h"
#include "Database/Database.h"
#include "Database/Instance.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMovieResourceFactory", FlashMovieResourceFactory, resource::ResourceFactory)

FlashMovieResourceFactory::FlashMovieResourceFactory(db::Database* db)
:	m_db(db)
{
}

const TypeSet FlashMovieResourceFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< FlashMovie >());
	return typeSet;
}

Object* FlashMovieResourceFactory::create(const Type& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< FlashMovieResource > resource = instance->checkout< FlashMovieResource >(db::CfReadOnly);
	if (!resource)
		return 0;

	Ref< Stream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	flash::SwfReader swf(stream);
	return flash::FlashMovieFactory().createMovie(&swf);
}

	}
}
