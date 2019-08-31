#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Shape/Editor/Traverser.h"
#include "World/EntityData.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.Traverser", Traverser, Object)

Traverser::Traverser(ISerializable* object)
:   m_object(object)
{
}

void Traverser::visit(const std::function< bool(Ref< world::EntityData >&) >& visitor)
{
	Ref< Reflection > reflection = Reflection::create(m_object);

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
				Traverser(entityData).visit(visitor);

			if (entityData != objectMember->get())
			{
				objectMember->set(entityData);
				needToApply = true;
			}
		}
		else if (objectMember->get())
			Traverser(objectMember->get()).visit(visitor);
	}

	if (needToApply)
		reflection->apply(m_object);
}

    }
}
