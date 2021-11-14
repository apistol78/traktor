#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Serialization/ISerializable.h"
#include "World/Editor/ResolveExternal.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor
{
	namespace world
	{

Ref< ISerializable > resolveExternal(
	const std::function< Ref< const ISerializable >(const Guid& objectId) >& getObjectFn,
	const ISerializable* object,
	const Guid& seed,
	AlignedVector< Guid >* outExternalEntities
)
{
	Ref< Reflection > reflection = Reflection::create(object);
	if (!reflection)
		return nullptr;

 	RefArray< ReflectionMember > objectMembers;
 	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	for (auto member : objectMembers)
	{
		RfmObject* objectMember = mandatory_non_null_type_cast< RfmObject* >(member.ptr());
		if (auto externalEntityDataRef = dynamic_type_cast< const world::ExternalEntityData* >(objectMember->get()))
		{
			Ref< const ISerializable > externalEntityData = getObjectFn(externalEntityDataRef->getEntityData());
			if (!externalEntityData)
				return nullptr;

			if (outExternalEntities)
				outExternalEntities->push_back(externalEntityDataRef->getEntityData());

			Guid entityDataId = externalEntityDataRef->getId().permutation(seed);

			Ref< world::EntityData > resolvedEntityData = dynamic_type_cast< world::EntityData* >(resolveExternal(getObjectFn, externalEntityData, entityDataId, outExternalEntities));
			if (!resolvedEntityData)
				return nullptr;

			resolvedEntityData->setId(entityDataId);
			resolvedEntityData->setName(externalEntityDataRef->getName());
			resolvedEntityData->setTransform(externalEntityDataRef->getTransform());

			for (auto componentData : externalEntityDataRef->getComponents())
				resolvedEntityData->setComponent(componentData);			

			objectMember->set(resolvedEntityData);
		}
		else if (auto entityDataRef = dynamic_type_cast< const world::EntityData* >(objectMember->get()))
		{
			Guid entityDataId = entityDataRef->getId().permutation(seed);

			Ref< world::EntityData > resolvedEntityData = dynamic_type_cast< world::EntityData* >(resolveExternal(getObjectFn, entityDataRef, entityDataId, outExternalEntities));
			if (!resolvedEntityData)
				return nullptr;

			resolvedEntityData->setId(entityDataId);
			objectMember->set(resolvedEntityData);
		}
		else if (objectMember->get())
		{
			objectMember->set(resolveExternal(getObjectFn, objectMember->get(), seed, outExternalEntities));
		}
	}

	return reflection->clone();
}

	}
}
