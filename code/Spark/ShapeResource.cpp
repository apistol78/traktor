#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Render/Shader.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/RenderMeshFactory.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spark/Shape.h"
#include "Spark/ShapeResource.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{
		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.ShapeResource", 0, ShapeResource, ISerializable)

Ref< Shape > ShapeResource::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, db::Instance* resourceInstance) const
{
	// Open stream to resource's mesh data.
	Ref< IStream > dataStream = resourceInstance->readData(L"Data");
	if (!dataStream)
	{
		log::error << L"Shape create failed; unable to open data stream" << Endl;
		return 0;
	}

	// Create renderable mesh.
	render::RenderMeshFactory meshFactory(renderSystem);
	Ref< render::Mesh > mesh = render::MeshReader(&meshFactory).read(dataStream);
	if (!mesh)
	{
		log::error << L"Shape create failed; unable to read render mesh" << Endl;
		return 0;
	}

	// Bind shaders.
	AlignedVector< Shape::Part > parts(m_parts.size());
	for (size_t i = 0; i < m_parts.size(); ++i)
	{
		if (!resourceManager->bind(m_parts[i].shader, parts[i].shader))
		{
			log::error << L"Shape create failed; unable to bind shader" << Endl;
			return 0;
		}
	}

	return new Shape(mesh, parts, m_bounds);
}

void ShapeResource::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Part, MemberComposite< Part > >(L"parts", m_parts);
	s >> MemberAabb2(L"bounds", m_bounds);
}

void ShapeResource::Part::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", shader);
}

	}
}
