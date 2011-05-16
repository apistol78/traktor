#include "Core/Containers/AlignedVector.h"
#include "Core/Io/MemoryStream.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieFactory.h"
#include "Flash/FlashMovieResource.h"
#include "Flash/FlashMovieResourceFactory.h"
#include "Flash/SwfReader.h"

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

bool FlashMovieResourceFactory::isCacheable() const
{
	return true;
}

Ref< Object > FlashMovieResourceFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< FlashMovieResource > resource = instance->getObject< FlashMovieResource >();
	if (!resource)
		return 0;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	uint32_t assetSize = stream->available();
	AlignedVector< uint8_t > assetBlob(assetSize);

	uint32_t offset = 0;
	while (offset < assetSize)
	{
		int nread = stream->read(&assetBlob[offset], assetSize - offset);
		if (nread < 0)
			return false;
		offset += nread;
	}

	stream->close();

	Ref< MemoryStream > memoryStream = new MemoryStream(&assetBlob[0], int(assetSize), true, false);
	Ref< SwfReader > swf = new SwfReader(memoryStream);

	Ref< FlashMovie > movie = flash::FlashMovieFactory().createMovie(swf);
	return movie;
}

	}
}
