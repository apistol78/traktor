#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Mesh/Lod/LodMeshEntity.h"
#include "Mesh/Lod/LodMeshEntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.LodMeshEntityData", 0, LodMeshEntityData, AbstractMeshEntityData)

LodMeshEntityData::LodMeshEntityData()
:	m_lodDistance(1.0f)
,	m_lodCullDistance(0.0f)
{
}

Ref< MeshEntity > LodMeshEntityData::createEntity(resource::IResourceManager* resourceManager, const world::IEntityBuilder* builder) const
{
	RefArray< MeshEntity > lods;

	// Create lod mesh entities.
	for (RefArray< AbstractMeshEntityData >::const_iterator i = m_lods.begin(); i != m_lods.end(); ++i)
	{
		Ref< MeshEntity > lod = dynamic_type_cast< MeshEntity* >(builder->create(*i));
		if (lod)
			lods.push_back(lod);
		else
		{
			log::error << L"Unable to create mesh entity from instance \"" << (*i)->getName() << L"\"" << Endl;
			return 0;
		}
	}

	// Must contain at least one lod.
	if (lods.empty())
		return 0;

	// Create lod mesh.
	return new LodMeshEntity(
		getTransform(),
		lods,
		m_lodDistance,
		m_lodCullDistance
	);
}

void LodMeshEntityData::setTransform(const Transform& transform)
{
	Transform deltaTransform = getTransform().inverse() * transform;
	for (RefArray< AbstractMeshEntityData >::iterator i = m_lods.begin(); i != m_lods.end(); ++i)
	{
		Transform currentTransform = (*i)->getTransform();
		(*i)->setTransform(currentTransform * deltaTransform);
	}
	AbstractMeshEntityData::setTransform(transform);
}

void LodMeshEntityData::serialize(ISerializer& s)
{
	AbstractMeshEntityData::serialize(s);
	
	s >> MemberRefArray< AbstractMeshEntityData >(L"lods", m_lods);
	s >> Member< float >(L"lodDistance", m_lodDistance, AttributeRange(0.0f));
	s >> Member< float >(L"lodCullDistance", m_lodCullDistance, AttributeRange(0.0f));
}

	}
}
