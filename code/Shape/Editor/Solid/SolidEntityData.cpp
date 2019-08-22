#include "Core/Serialization/ISerializer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/SolidEntity.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.SolidEntityData", 0, SolidEntityData, world::GroupEntityData)

Ref< SolidEntity > SolidEntityData::createEntity(const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;

    Ref< SolidEntity > solidEntity = new SolidEntity(renderSystem, shader, getTransform());

    for (auto childEntityData : getEntityData())
    {
        Ref< PrimitiveEntity > primitiveEntity = builder->create< PrimitiveEntity >(childEntityData);
        if (primitiveEntity)
            solidEntity->addEntity(primitiveEntity);
    }

	return solidEntity;
}

void SolidEntityData::serialize(ISerializer& s)
{
	world::GroupEntityData::serialize(s);
	s >> resource::Member< render::Shader >(L"shader", m_shader);
}

    }
}