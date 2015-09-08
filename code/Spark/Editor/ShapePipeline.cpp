#pragma optimize( "", off )

#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Format.h"
#include "Core/Math/Triangulator.h"
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
#include "Spark/Editor/ShapeAsset.h"
#include "Spark/Editor/ShapePipeline.h"
#include "Spark/Editor/Shape/Shape.h"
#include "Spark/Editor/Shape/Style.h"
#include "Spark/Editor/Shape/SvgParser.h"
#include "Spark/Editor/Shape/TesselatorVisitor.h"
#include "Xml/Document.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

class TriangleProducer : public TesselatorVisitor::Listener
{
public:
	struct Batch
	{
		AlignedVector< Vector2 > vertices;
		AlignedVector< uint32_t > triangles;
	};

	virtual void drawShape(
		const Style* style,
		const AlignedVector< Vector2 >& shape
	) T_FINAL
	{
		if (style->getFillEnable())
		{
			// Create triangles from possibly complex shapes.
			Triangulator().freeze(shape, m_triangles);
			if (!m_triangles.empty())
			{
				// Batch triangles by style.
				Batch& batch = m_batches[style];

				uint32_t vertexBase = uint32_t(batch.vertices.size());
				batch.vertices.insert(batch.vertices.end(), shape.begin(), shape.end());

				for (std::vector< Triangulator::Triangle >::const_iterator i = m_triangles.begin(); i != m_triangles.end(); ++i)
				{
					batch.triangles.push_back(vertexBase + uint32_t(i->indices[0]));
					batch.triangles.push_back(vertexBase + uint32_t(i->indices[1]));
					batch.triangles.push_back(vertexBase + uint32_t(i->indices[2]));
				}

				m_triangles.resize(0);
			}
		}
	}

	const std::map< const Style*, Batch >& getBatches() const { return m_batches; }

private:
	std::vector< Triangulator::Triangle > m_triangles;
	std::map< const Style*, Batch > m_batches;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.ShapePipeline", 0, ShapePipeline, editor::IPipeline)

ShapePipeline::ShapePipeline()
{
}

bool ShapePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void ShapePipeline::destroy()
{
}

TypeInfoSet ShapePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ShapeAsset >());
	return typeSet;
}

bool ShapePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const ShapeAsset* shapeAsset = checked_type_cast< const ShapeAsset* >(sourceAsset);
	pipelineDepends->addDependency(traktor::Path(m_assetPath), shapeAsset->getFileName().getOriginal());
	pipelineDepends->addDependency(shapeAsset->m_shader, editor::PdfBuild | editor::PdfResource);
	return true;
}

bool ShapePipeline::buildOutput(
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
	const ShapeAsset* shapeAsset = checked_type_cast< const ShapeAsset* >(sourceAsset);

	// Open stream to source file.
	Ref< IStream > sourceStream = pipelineBuilder->openFile(traktor::Path(m_assetPath), shapeAsset->getFileName().getOriginal());
	if (!sourceStream)
	{
		log::error << L"Shape pipeline failed; unable to open file \"" << shapeAsset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	// Parse XML document containing SVG.
	xml::Document document;
	if (!document.loadFromStream(sourceStream))
	{
		log::error << L"Shape pipeline failed; unable to parse XML file \"" << shapeAsset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	sourceStream->close();
	sourceStream = 0;

	// Parse SVG into intermediate shape.
	Ref< Shape > shape = SvgParser().parse(&document);
	if (!shape)
	{
		log::error << L"Shape pipeline failed; unable to parse SVG file \"" << shapeAsset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	// Convert intermediate shape into a set of triangles.
	TriangleProducer triangleProducer;
	TesselatorVisitor visitor(triangleProducer);
	shape->visit(&visitor);

	// Create shape output resource.
	Ref< ShapeResource > outputShapeResource = new ShapeResource();
	outputShapeResource->m_shader = shapeAsset->m_shader;
	
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

	// Create render mesh from triangles and write to data stream.
	const std::map< const Style*, TriangleProducer::Batch >& batches = triangleProducer.getBatches();

	// Count number of vertices and indices.
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;

	for (std::map< const Style*, TriangleProducer::Batch >::const_iterator i = batches.begin(); i != batches.end(); ++i)
	{
		vertexCount += uint32_t(i->second.vertices.size());
		indexCount += uint32_t(i->second.triangles.size() * 3);
	}

	// Measure shape bounds.
	Aabb2 boundingBox;
	for (std::map< const Style*, TriangleProducer::Batch >::const_iterator i = batches.begin(); i != batches.end(); ++i)
	{
		for (AlignedVector< Vector2 >::const_iterator j = i->second.vertices.begin(); j != i->second.vertices.end(); ++j)
		{
			T_FATAL_ASSERT(!isNanOrInfinite(j->x));
			T_FATAL_ASSERT(!isNanOrInfinite(j->y));
			boundingBox.contain(*j);
		}
	}

	// Define shape render vertex.
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, 0));

	Ref< render::Mesh > renderMesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		vertexCount * render::getVertexSize(vertexElements),
		render::ItUInt16,
		indexCount * sizeof(uint16_t)
	);

	// Fill vertices.
	float* vertex = static_cast< float* >(renderMesh->getVertexBuffer()->lock());
	for (std::map< const Style*, TriangleProducer::Batch >::const_iterator i = batches.begin(); i != batches.end(); ++i)
	{
		for (AlignedVector< Vector2 >::const_iterator j = i->second.vertices.begin(); j != i->second.vertices.end(); ++j)
		{
			T_FATAL_ASSERT(!isNanOrInfinite(j->x));
			T_FATAL_ASSERT(!isNanOrInfinite(j->y));
			*vertex++ = j->x - boundingBox.getCenter().x;
			*vertex++ = j->y - boundingBox.getCenter().y;
		}
	}
	renderMesh->getVertexBuffer()->unlock();

	// Fill indices.
	uint16_t* index = static_cast< uint16_t* >(renderMesh->getIndexBuffer()->lock());
	for (std::map< const Style*, TriangleProducer::Batch >::const_iterator i = batches.begin(); i != batches.end(); ++i)
	{
		for (AlignedVector< uint32_t >::const_iterator j = i->second.triangles.begin(); j != i->second.triangles.end(); ++j)
			*index++ = *j;
	}
	renderMesh->getIndexBuffer()->unlock();

	// Setup mesh parts.
	std::vector< render::Mesh::Part > meshParts;

	uint32_t indexOffset = 0;
	for (std::map< const Style*, TriangleProducer::Batch >::const_iterator i = batches.begin(); i != batches.end(); ++i)
	{
		render::Mesh::Part part;
		part.name = L"";
		part.primitives.setIndexed(
			render::PtTriangles,
			indexOffset,
			uint32_t(i->second.triangles.size()),
			0,
			vertexCount - 1
		);
		meshParts.push_back(part);
		indexOffset += uint32_t(i->second.triangles.size() * 3);
	}

	renderMesh->setParts(meshParts);

	// No bounding box used.
	renderMesh->setBoundingBox(Aabb3());

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

Ref< ISerializable > ShapePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
