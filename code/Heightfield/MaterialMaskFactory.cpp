#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Heightfield/MaterialMask.h"
#include "Heightfield/MaterialMaskFactory.h"
#include "Heightfield/MaterialMaskResource.h"
#include "Heightfield/MaterialMaskResourceLayer.h"
#include "Heightfield/MaterialParams.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.MaterialMaskFactory", MaterialMaskFactory, resource::IResourceFactory)

MaterialMaskFactory::MaterialMaskFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet MaterialMaskFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MaterialMaskResource >());
	return typeSet;
}

const TypeInfoSet MaterialMaskFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MaterialMask >());
	return typeSet;
}

bool MaterialMaskFactory::isCacheable() const
{
	return true;
}

Ref< Object > MaterialMaskFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< MaterialMaskResource > resource = instance->getObject< MaterialMaskResource >();
	if (!resource)
		return 0;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	const RefArray< MaterialMaskResourceLayer >& resourceLayers = resource->getLayers();

	RefArray< MaterialParams > params(resourceLayers.size());
	for (uint32_t i = 0; i < resourceLayers.size(); ++i)
	{
		params[i] = new MaterialParams();

		const RefArray< ISerializable >& resourceLayerParams = resourceLayers[i]->getParams();
		for (RefArray< ISerializable >::const_iterator j = resourceLayerParams.begin(); j != resourceLayerParams.end(); ++j)
			params[i]->set(*j);
	}

	uint32_t size = resource->getSize();
	
	Ref< MaterialMask > mask = new MaterialMask(size, params);
	Reader(stream).read(mask->m_data.ptr(), size * size, sizeof(uint8_t));

	stream->close();

	return mask;
}

	}
}
