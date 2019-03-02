#include <cstring>
#include <list>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Bezier3rd.h"
#include "Core/Math/Format.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Render/IndexBuffer.h"
#include "Render/VertexBuffer.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Spark/ShapeResource.h"
#include "Spark/Triangulator.h"
#include "Spark/Editor/VectorShape.h"
#include "Spark/Editor/VectorShapeAsset.h"
#include "Spark/Editor/VectorShapeFormat.h"
#include "Spark/Editor/VectorShapePipeline.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

const float c_controlPoints[3][2] =
{
	{ 0.0f, 0.0f },
	{ 0.5f, 0.0f },
	{ 1.0f, 1.0f }
};

#pragma pack(1)
struct Vertex
{
	float position[2];
	float texCoord[2];
	float controlPoints[2];
};
#pragma pack()

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.VectorShapePipeline", 0, VectorShapePipeline, editor::IPipeline)

VectorShapePipeline::VectorShapePipeline()
{
}

bool VectorShapePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

void VectorShapePipeline::destroy()
{
}

TypeInfoSet VectorShapePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< VectorShapeAsset >());
	return typeSet;
}

bool VectorShapePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const VectorShapeAsset* shapeAsset = checked_type_cast< const VectorShapeAsset* >(sourceAsset);
	pipelineDepends->addDependency(traktor::Path(m_assetPath), shapeAsset->getFileName().getOriginal());
	pipelineDepends->addDependency(Guid(L"{E411A034-2FDA-4B44-A378-700D1CB8B6E4}"), editor::PdfBuild | editor::PdfResource);
	return true;
}

bool VectorShapePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const VectorShapeAsset* shapeAsset = checked_type_cast< const VectorShapeAsset* >(sourceAsset);

	// Open stream to source file.
	Ref< IStream > sourceStream = pipelineBuilder->openFile(traktor::Path(m_assetPath), shapeAsset->getFileName().getOriginal());
	if (!sourceStream)
	{
		log::error << L"Shape pipeline failed; unable to open file \"" << shapeAsset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	Ref< VectorShape > shape = VectorShapeFormat::readAny(sourceStream, shapeAsset->getFileName().getExtension());
	if (!shape)
	{
		log::error << L"Shape pipeline failed; unable to read file \"" << shapeAsset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	sourceStream->close();
	sourceStream = 0;

	// Create shape output resource.
	Ref< ShapeResource > outputShapeResource = new ShapeResource();

	// Set shader resource.
	outputShapeResource->m_shader = resource::Id< render::Shader >(Guid(L"{E411A034-2FDA-4B44-A378-700D1CB8B6E4}"));

	// Generate vertices and parts.
	AlignedVector< Vertex > vertices;
	std::vector< render::Mesh::Part > meshParts;

	const AlignedVector< VectorShape::PathWithStyle >& paths = shape->getPaths();
	for (AlignedVector< VectorShape::PathWithStyle >::const_iterator i = paths.begin(); i != paths.end(); ++i)
	{
		// \fixme Only fills are currently implemented.
		if (i->fillStyle < 0)
			continue;

		AlignedVector< Triangulator::Triangle > triangles;
		Triangulator().fill(&i->path, triangles);

		for (int32_t type = 0; type < 3; ++type)
		{
			uint32_t vertexOffset = vertices.size();

			for (AlignedVector< Triangulator::Triangle >::const_iterator j = triangles.begin(); j != triangles.end(); ++j)
			{
				if (j->type != (Triangulator::TriangleType)type)
					continue;

				for (int32_t k = 0; k < 3; ++k)
				{
					Vertex v;
					v.position[0] = j->v[k].x;
					v.position[1] = j->v[k].y;
					v.texCoord[0] = 0.0f;
					v.texCoord[1] = 0.0f;
					v.controlPoints[0] = c_controlPoints[k][0];
					v.controlPoints[1] = c_controlPoints[k][1];
					vertices.push_back(v);
					outputShapeResource->m_bounds.contain(j->v[k]);
				}
			}

			uint32_t vertexCount = vertices.size() - vertexOffset;
			uint32_t triangleCount = vertexCount / 3;

			if (triangleCount > 0)
			{
				// Setup render mesh part.
				render::Mesh::Part part;
				part.primitives.setNonIndexed(
					render::PtTriangles,
					vertexOffset,
					triangleCount
				);
				meshParts.push_back(part);

				// Setup shape part.
				const int8_t c_curveSigns[] = { 0, 1, -1 };
				ShapeResource::Part shapePart;
				shapePart.fillColor = shape->getFillStyle(i->fillStyle).color;
				shapePart.curveSign = c_curveSigns[type];
				outputShapeResource->m_parts.push_back(shapePart);
			}
		}
	}

	// Adjust for pivot.
	Vector2 offset = Vector2::zero();
	switch (shapeAsset->m_pivot)
	{
	default:
	case VectorShapeAsset::PtViewTopLeft:
		break;

	case VectorShapeAsset::PtViewCenter:
		offset = -shape->getBounds().getCenter();
		break;

	case VectorShapeAsset::PtShapeCenter:
		offset = -outputShapeResource->m_bounds.getCenter();
		break;
	}

	outputShapeResource->m_bounds.mn += offset;
	outputShapeResource->m_bounds.mx += offset;

	for (AlignedVector< Vertex >::iterator i = vertices.begin(); i != vertices.end(); ++i)
	{
		i->position[0] += offset.x;
		i->position[1] += offset.y;
	}

	// Create render mesh.
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, position)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, texCoord), 0));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, controlPoints), 1));

	Ref< render::Mesh > renderMesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		vertices.size() * render::getVertexSize(vertexElements),
		render::ItUInt16,
		0
	);

	void* vertexBufferPtr = renderMesh->getVertexBuffer()->lock();
	std::memcpy(
		vertexBufferPtr,
		vertices.c_ptr(),
		vertices.size() * render::getVertexSize(vertexElements)
	);
	renderMesh->getVertexBuffer()->unlock();

	renderMesh->setParts(meshParts);
	renderMesh->setBoundingBox(Aabb3());

	// Create output instance.
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!outputInstance)
	{
		log::error << L"Shape pipeline failed; unable to create output instance" << Endl;
		return false;
	}

	// Open asset data stream.
	Ref< IStream > dataStream = outputInstance->writeData(L"Data");
	if (!dataStream)
	{
		log::error << L"Shape pipeline failed; unable to create mesh data stream" << Endl;
		outputInstance->revert();
		return false;
	}

	// Write mesh to data stream.
	if (!render::MeshWriter().write(dataStream, renderMesh))
		return false;

	// Commit resource.
	outputInstance->setObject(outputShapeResource);
	if (!outputInstance->commit())
	{
		log::error << L"Shape pipeline failed; unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > VectorShapePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
