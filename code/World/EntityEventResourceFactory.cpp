/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Instance.h"
#include "World/EntityEventResourceFactory.h"
#include "World/EntityEventSet.h"
#include "World/EntityEventSetData.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityEventResourceFactory", EntityEventResourceFactory, resource::IResourceFactory)

EntityEventResourceFactory::EntityEventResourceFactory(const IEntityBuilder* entityBuilder)
:	m_entityBuilder(entityBuilder)
{
}

const TypeInfoSet EntityEventResourceFactory::getResourceTypes() const
{
	return makeTypeInfoSet< EntityEventSetData, IEntityEventData >();
}

const TypeInfoSet EntityEventResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	if (is_type_a< EntityEventSetData >(resourceType))
		return makeTypeInfoSet< EntityEventSet >();
	else if (is_type_a< IEntityEventData >(resourceType))
		return makeTypeInfoSet< IEntityEvent >();
	else
		return TypeInfoSet();
}

bool EntityEventResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > EntityEventResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	if (is_type_a< EntityEventSet >(productType))
	{
		Ref< const EntityEventSetData > eventSetData = instance->getObject< EntityEventSetData >();
		if (!eventSetData)
			return 0;

		Ref< EntityEventSet > eventSet = eventSetData->create(m_entityBuilder);
		if (!eventSet)
			return 0;

		return eventSet;
	}
	else if (is_type_a< IEntityEvent >(productType))
	{
		Ref< const IEntityEventData > eventData = instance->getObject< IEntityEventData >();
		if (!eventData)
			return 0;

		Ref< IEntityEvent > event = m_entityBuilder->create< IEntityEvent >(eventData);
		if (!event)
			return 0;

		return event;
	}
	else
		return 0;
}

	}
}
