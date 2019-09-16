#include "Scene/Editor/IEntityReplicator.h"

namespace traktor
{
    namespace scene
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.IEntityReplicator", IEntityReplicator, Object)

Ref< const IEntityReplicator > IEntityReplicator::createEntityReplicator(const TypeInfo& dataType)
{
	TypeInfoSet entityReplicatorTypes;
	type_of< IEntityReplicator >().findAllOf(entityReplicatorTypes, false);

	for (const auto& entityReplicatorType : entityReplicatorTypes)
	{
		Ref< IEntityReplicator > entityReplicator = mandatory_non_null_type_cast< IEntityReplicator* >(entityReplicatorType->createInstance());
		auto supportedTypes = entityReplicator->getSupportedTypes();
		if (supportedTypes.find(&dataType) != supportedTypes.end())
			return entityReplicator;
	}

    return nullptr;
}

    }
}
