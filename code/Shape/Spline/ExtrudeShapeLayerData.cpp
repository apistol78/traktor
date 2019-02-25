#include "Core/Serialization/ISerializer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Shape/Spline/ExtrudeShapeLayer.h"
#include "Shape/Spline/ExtrudeShapeLayerData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.ExtrudeShapeLayerData", 0, ExtrudeShapeLayerData, ISplineLayerData)

ExtrudeShapeLayerData::ExtrudeShapeLayerData()
:	m_automaticOrientation(false)
,	m_detail(10.0f)
{
}

Ref< ISplineLayer > ExtrudeShapeLayerData::createLayer(SplineEntity* owner, const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;

	return new ExtrudeShapeLayer(owner, renderSystem, shader, m_automaticOrientation, m_detail);
}

void ExtrudeShapeLayerData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< bool >(L"automaticOrientation", m_automaticOrientation);
	s >> Member< float >(L"detail", m_detail);
}

	}
}
