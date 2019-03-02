#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Mesh/Static/StaticMesh.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Weather/Precipitation/PrecipitationComponent.h"
#include "Weather/Precipitation/PrecipitationComponentData.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.weather.PrecipitationComponentData", 0, PrecipitationComponentData, world::IEntityComponentData)

PrecipitationComponentData::PrecipitationComponentData()
:	m_tiltRate(6.0f)
,	m_parallaxDistance(1.0f)
,	m_depthDistance(1.0f)
,	m_opacity(0.1f)
{
}

Ref< PrecipitationComponent > PrecipitationComponentData::createComponent(resource::IResourceManager* resourceManager) const
{
	resource::Proxy< mesh::StaticMesh > mesh;
	if (!resourceManager->bind(m_mesh, mesh))
		return 0;

	return new PrecipitationComponent(mesh, m_tiltRate, m_parallaxDistance, m_depthDistance, m_opacity);
}

void PrecipitationComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< mesh::StaticMesh >(L"mesh", m_mesh);
	s >> Member< float >(L"tiltRate", m_tiltRate, AttributeRange(0.0f));
	s >> Member< float >(L"parallaxDistance", m_parallaxDistance, AttributeRange(0.0f));
	s >> Member< float >(L"depthDistance", m_depthDistance, AttributeRange(0.0f));
	s >> Member< float >(L"opacity", m_opacity, AttributeRange(0.0f));
}

	}
}
