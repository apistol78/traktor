#include "Core/Io/Reader.h"
#include "Database/Instance.h"
#include "Spray/Effect.h"
#include "Spray/EffectData.h"
#include "Spray/EffectFactory.h"
#include "Spray/PointSet.h"
#include "Spray/PointSetResource.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectFactory", EffectFactory, resource::IResourceFactory)

EffectFactory::EffectFactory(const world::IEntityBuilder* entityBuilder)
:	m_entityBuilder(entityBuilder)
{
}

const TypeInfoSet EffectFactory::getResourceTypes() const
{
	return makeTypeInfoSet< EffectData, PointSetResource >();
}

const TypeInfoSet EffectFactory::getProductTypes(const TypeInfo& resourceType) const
{
	if (is_type_a< EffectData >(resourceType))
		return makeTypeInfoSet< Effect >();
	else if (is_type_a< PointSetResource >(resourceType))
		return makeTypeInfoSet< PointSet >();
	else
		return TypeInfoSet();
}

bool EffectFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > EffectFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	if (is_type_a< Effect >(productType))
	{
		Ref< EffectData > effectData = instance->getObject< EffectData >();
		if (effectData)
			return effectData->createEffect(resourceManager, m_entityBuilder);
		else
			return 0;
	}
	else if (is_type_a< PointSet >(productType))
	{
		Ref< IStream > stream = instance->readData(L"Data");
		if (!stream)
			return 0;
			
		Ref< PointSet > pointSet = new PointSet();
		if (!pointSet->read(stream))
			return 0;

		return pointSet;
	}
	else
		return 0;
}

	}
}
