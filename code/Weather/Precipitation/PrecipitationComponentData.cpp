#include "Core/Serialization/ISerializer.h"
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
{
}

Ref< PrecipitationComponent > PrecipitationComponentData::createComponent(resource::IResourceManager* resourceManager) const
{
	resource::Proxy< mesh::StaticMesh > mesh;
	if (!resourceManager->bind(m_mesh, mesh))
		return 0;

	return new PrecipitationComponent(mesh);
}

void PrecipitationComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< mesh::StaticMesh >(L"mesh", m_mesh);
}

	}
}
