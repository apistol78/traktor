#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
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

	// Bind runtime shader.
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
	{
		log::error << L"Shape create failed; unable to bind shader" << Endl;
		return 0;
	}

	return new Shape(mesh, shader, m_parts);
}

void ShapeResource::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberStlVector< uint8_t >(L"parts", m_parts);
}

	}
}
