#pragma optimize( "", off )

#include <list>

#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Format.h"
//#include "Core/Math/Triangulator.h"
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
#include "Spark/Editor/Shape/PathShape.h"
#include "Spark/Editor/Shape/Shape.h"
#include "Spark/Editor/Shape/ShapeVisitor.h"
#include "Spark/Editor/Shape/Style.h"
#include "Spark/Editor/Shape/SvgParser.h"
#include "Spark/Editor/Shape/Triangulator.h"
#include "Xml/Document.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

class TriangleProducer : public ShapeVisitor
{
public:
	Ref< const Style > m_style;

	struct Batch
	{
		AlignedVector< Vector2 > vertices;
		AlignedVector< uint32_t > triangles;
	};

	virtual void enter(Shape* shape)
	{
		if (shape->getStyle())
			m_style = shape->getStyle();

		if (!m_style || !m_style->getFillEnable())
			return;

		PathShape* pathShape = dynamic_type_cast< PathShape* >(shape);
		if (pathShape)
		{
			AlignedVector< Triangulator::Segment > segments;

			const std::vector< SubPath >& subPaths = pathShape->getPath().getSubPaths();
			for (std::vector< SubPath >::const_iterator i = subPaths.begin(); i != subPaths.end(); ++i)
			{
				switch (i->type)
				{
				case SptLinear:
					{
						for (uint32_t j = 0; j < i->points.size() - 1; ++j)
						{
							Triangulator::Segment s;
							s.curve = false;
							s.v[0] = i->points[j];
							s.v[1] = i->points[j + 1];
							segments.push_back(s);
						}
						if (i->closed)
						{
							Triangulator::Segment s;
							s.curve = false;
							s.v[0] = i->points.back();
							s.v[1] = i->points.front();
							segments.push_back(s);
						}
					}
					break;

				case SptCubic:
					{
					}
					break;

				case SptQuadric:
					{
						for (uint32_t j = 0; j < i->points.size() - 1; j += 2)
						{
							Triangulator::Segment s;
							s.curve = true;
							s.v[0] = i->points[j];
							s.v[1] = i->points[j + 1];
							s.c = i->points[j + 2];
							segments.push_back(s);
						}
					}
					break;
				}
			}

			if (!segments.empty())
			{
				AlignedVector< Triangulator::Triangle > triangles;
				Triangulator().triangulate(segments, triangles);

				if (!triangles.empty())
				{
					Batch batch;

					for (AlignedVector< Triangulator::Triangle >::const_iterator i = triangles.begin(); i != triangles.end(); ++i)
					{
						uint32_t indexBase = batch.vertices.size();

						batch.vertices.push_back(i->v[0]);
						batch.vertices.push_back(i->v[1]);
						batch.vertices.push_back(i->v[2]);

						batch.triangles.push_back(indexBase + 0);
						batch.triangles.push_back(indexBase + 1);
						batch.triangles.push_back(indexBase + 2);
					}

					m_batches.push_back(std::make_pair(m_style, batch));
				}
			}
		}
	}

	virtual void leave(Shape* shape)
	{
	}

	const std::list< std::pair< const Style*, Batch > >& getBatches() const { return m_batches; }

private:
	std::list< std::pair< const Style*, Batch > > m_batches;
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
	shape->visit(&triangleProducer);

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
	const std::list< std::pair< const Style*, TriangleProducer::Batch > >& batches = triangleProducer.getBatches();

	// Count number of vertices and indices.
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;

	for (std::list< std::pair< const Style*, TriangleProducer::Batch > >::const_iterator i = batches.begin(); i != batches.end(); ++i)
	{
		vertexCount += uint32_t(i->second.vertices.size());
		indexCount += uint32_t(i->second.triangles.size() * 3);
	}

	// Measure shape bounds.
	Aabb2 boundingBox;
	for (std::list< std::pair< const Style*, TriangleProducer::Batch > >::const_iterator i = batches.begin(); i != batches.end(); ++i)
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
	vertexElements.push_back(render::VertexElement(render::DuColor, render::DtFloat4, 2 * sizeof(float)));

	Ref< render::Mesh > renderMesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		vertexCount * render::getVertexSize(vertexElements),
		render::ItUInt16,
		indexCount * sizeof(uint16_t)
	);

	// Fill vertices.
	float* vertex = static_cast< float* >(renderMesh->getVertexBuffer()->lock());
	for (std::list< std::pair< const Style*, TriangleProducer::Batch > >::const_iterator i = batches.begin(); i != batches.end(); ++i)
	{
		for (AlignedVector< Vector2 >::const_iterator j = i->second.vertices.begin(); j != i->second.vertices.end(); ++j)
		{
			T_FATAL_ASSERT(!isNanOrInfinite(j->x));
			T_FATAL_ASSERT(!isNanOrInfinite(j->y));
			*vertex++ = j->x - boundingBox.getCenter().x;
			*vertex++ = j->y - boundingBox.getCenter().y;

			*vertex++ = i->first->getFill().r / 255.0f;
			*vertex++ = i->first->getFill().g / 255.0f;
			*vertex++ = i->first->getFill().b / 255.0f;
			*vertex++ = i->first->getFill().a / 255.0f;
		}
	}
	renderMesh->getVertexBuffer()->unlock();

	// Fill indices.
	uint16_t* index = static_cast< uint16_t* >(renderMesh->getIndexBuffer()->lock());

	uint32_t vertexOffset = 0;
	for (std::list< std::pair< const Style*, TriangleProducer::Batch > >::const_iterator i = batches.begin(); i != batches.end(); ++i)
	{
		for (AlignedVector< uint32_t >::const_iterator j = i->second.triangles.begin(); j != i->second.triangles.end(); ++j)
			*index++ = vertexOffset + *j;
		vertexOffset += uint32_t(i->second.vertices.size());
	}
	renderMesh->getIndexBuffer()->unlock();

	// Setup mesh parts.
	std::vector< render::Mesh::Part > meshParts;

	uint32_t indexOffset = 0;
	for (std::list< std::pair< const Style*, TriangleProducer::Batch > >::const_iterator i = batches.begin(); i != batches.end(); ++i)
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
