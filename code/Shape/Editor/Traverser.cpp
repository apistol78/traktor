#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Shape/Editor/Traverser.h"
#include "World/EntityData.h"

namespace traktor
{
    namespace shape
    {

void Traverser::visit(const ISerializable* object, const std::function< bool(const world::EntityData*) >& visitor)
{
	Ref< Reflection > reflection = Reflection::create(object);

 	RefArray< ReflectionMember > objectMembers;
 	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	for (auto member : objectMembers)
	{
		RfmObject* objectMember = dynamic_type_cast< RfmObject* >(member);
		if (!objectMember->get())
			continue;

		const world::EntityData* entityData = dynamic_type_cast< const world::EntityData* >(objectMember->get());
		if (entityData)
		{
			if (visitor(entityData))
				Traverser::visit(entityData, visitor);
		}
		else if (objectMember->get())
			Traverser::visit(objectMember->get(), visitor);
	}
}

void Traverser::visit(ISerializable* object, const std::function< bool(Ref< world::EntityData >&) >& visitor)
{
	Ref< Reflection > reflection = Reflection::create(object);

 	RefArray< ReflectionMember > objectMembers;
 	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	bool needToApply = false;

	for (auto member : objectMembers)
	{
		RfmObject* objectMember = dynamic_type_cast< RfmObject* >(member);
		if (!objectMember->get())
			continue;

		Ref< world::EntityData > entityData = dynamic_type_cast< world::EntityData* >(objectMember->get());
		if (entityData)
		{
			if (visitor(entityData))
				Traverser::visit(entityData, visitor);

			if (entityData != objectMember->get())
			{
				objectMember->set(entityData);
				needToApply = true;
			}
		}
		else if (objectMember->get())
			Traverser::visit(objectMember->get(), visitor);
	}

	if (needToApply)
		reflection->apply(object);
}

    }
}
