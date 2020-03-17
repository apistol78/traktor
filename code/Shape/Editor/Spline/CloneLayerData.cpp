#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Shape/Editor/Spline/CloneLayer.h"
#include "Shape/Editor/Spline/CloneLayerData.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.CloneLayerData", 0, CloneLayerData, SplineLayerComponentData)

CloneLayerData::CloneLayerData()
:	m_distance(1.0f)
{
}

Ref< SplineLayerComponent > CloneLayerData::createComponent(const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	return new CloneLayer(
		builder->getCompositeEntityBuilder(),
		m_entity,
		m_distance
	);
}

void CloneLayerData::serialize(ISerializer& s)
{
	s >> MemberRef< world::EntityData >(L"entity", m_entity);
	s >> Member< float >(L"distance", m_distance, AttributeUnit(AuMetres));
}

	}
}
