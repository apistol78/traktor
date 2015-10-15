#include <list>
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
#include "Spark/Editor/VectorShapeAsset.h"
#include "Spark/Editor/VectorShapePipeline.h"
#include "Spark/Editor/Shape/SvgDocument.h"
#include "Spark/Editor/Shape/SvgParser.h"
#include "Spark/Editor/Shape/SvgPathShape.h"
#include "Spark/Editor/Shape/SvgShape.h"
#include "Spark/Editor/Shape/SvgShapeVisitor.h"
#include "Spark/Editor/Shape/SvgStyle.h"
#include "Xml/Document.h"

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

class TriangleProducer : public SvgShapeVisitor
{
public:
	struct Batch
	{
		AlignedVector< Vector2 > vertices;
		AlignedVector< uint32_t > trianglesFill;
		AlignedVector< uint32_t > trianglesIn;
		AlignedVector< uint32_t > trianglesOut;
	};

	TriangleProducer()
	:	m_viewBox(Vector2(0.0f, 0.0f), Vector2(100.0f, 100.0f))
	,	m_size(100.0f, 100.0f)
	{
		m_transformStack.push_back(Matrix33::identity());
	}

	virtual void enter(SvgShape* shape)
	{
		m_transformStack.push_back(m_transformStack.back() * shape->getTransform());
		const Matrix33& T = m_transformStack.back();

		SvgDocument* document = dynamic_type_cast< SvgDocument* >(shape);
		if (document)
		{
			m_viewBox = document->getViewBox();
			m_size = document->getSize();
		}

		if (shape->getStyle())
			m_styleStack.push_back(shape->getStyle());

		SvgPathShape* pathShape = dynamic_type_cast< SvgPathShape* >(shape);
		if (pathShape)
		{
			if (m_styleStack.empty() || !m_styleStack.back()->getFillEnable())
				return;

			AlignedVector< Triangulator::Triangle > triangles;
			Triangulator().fill(&pathShape->getPath(), triangles);

			if (!triangles.empty())
			{
				Batch* batch = 0;

				// Check if we can merge with last batch.
				if (!m_batches.empty() && *m_batches.back().first == *m_styleStack.back())
				{
					batch = &m_batches.back().second;
				}
				if (!batch)
				{
					m_batches.push_back(std::make_pair(m_styleStack.back(), Batch()));
					batch = &m_batches.back().second;
				}

				for (AlignedVector< Triangulator::Triangle >::const_iterator i = triangles.begin(); i != triangles.end(); ++i)
				{
					uint32_t indexBase = batch->vertices.size();

					batch->vertices.push_back(i->v[0].toVector2());
					batch->vertices.push_back(i->v[1].toVector2());
					batch->vertices.push_back(i->v[2].toVector2());

					if (i->type == Triangulator::TcFill)
					{
						batch->trianglesFill.push_back(indexBase + 0);
						batch->trianglesFill.push_back(indexBase + 1);
						batch->trianglesFill.push_back(indexBase + 2);
					}
					else if (i->type == Triangulator::TcIn)
					{
						batch->trianglesIn.push_back(indexBase + 0);
						batch->trianglesIn.push_back(indexBase + 1);
						batch->trianglesIn.push_back(indexBase + 2);
					}
					else if (i->type == Triangulator::TcOut)
					{
						batch->trianglesOut.push_back(indexBase + 0);
						batch->trianglesOut.push_back(indexBase + 1);
						batch->trianglesOut.push_back(indexBase + 2);
					}
				}
			}
		}
	}

	virtual void leave(SvgShape* shape)
	{
		m_transformStack.pop_back();
		if (shape->getStyle())
			m_styleStack.pop_back();
	}

	const Aabb2& getViewBox() const { return m_viewBox; }

	const std::list< std::pair< const SvgStyle*, Batch > >& getBatches() const { return m_batches; }

private:
	std::list< std::pair< const SvgStyle*, Batch > > m_batches;
	Aabb2 m_viewBox;
	Vector2 m_size;
	RefArray< const SvgStyle > m_styleStack;
	AlignedVector< Matrix33 > m_transformStack;
	Matrix33 m_currentTransform;
};

Color4f toColor4f(const SvgStyle* style)
{
	return Color4f(
		style->getFill().r / 255.0f,
		style->getFill().g / 255.0f,
		style->getFill().b / 255.0f,
		int32_t(style->getOpacity() * 255)
	);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.VectorShapePipeline", 0, VectorShapePipeline, editor::IPipeline)

VectorShapePipeline::VectorShapePipeline()
{
}

bool VectorShapePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
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
	Ref< SvgShape > shape = SvgParser().parse(&document);
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

	// Set shader resource.
	outputShapeResource->m_shader = resource::Id< render::Shader >(Guid(L"{E411A034-2FDA-4B44-A378-700D1CB8B6E4}"));

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
	const Aabb2& viewBox = triangleProducer.getViewBox();
	const std::list< std::pair< const SvgStyle*, TriangleProducer::Batch > >& batches = triangleProducer.getBatches();

	// Count total number of triangles.
	uint32_t triangleCount = 0;
	for (std::list< std::pair< const SvgStyle*, TriangleProducer::Batch > >::const_iterator i = batches.begin(); i != batches.end(); ++i)
	{
		triangleCount += uint32_t(i->second.trianglesFill.size() / 3);
		triangleCount += uint32_t(i->second.trianglesIn.size() / 3);
		triangleCount += uint32_t(i->second.trianglesOut.size() / 3);
	}
	if (!triangleCount)
	{
		log::error << L"Shape pipeline failed; no geometry" << Endl;
		outputInstance->revert();
		return false;
	}

	// Measure initial shape bounds.
	Aabb2 bounds;
	for (std::list< std::pair< const SvgStyle*, TriangleProducer::Batch > >::const_iterator i = batches.begin(); i != batches.end(); ++i)
	{
		for (AlignedVector< Vector2 >::const_iterator j = i->second.vertices.begin(); j != i->second.vertices.end(); ++j)
			bounds.contain(*j);
	}

	// Determine offset to place pivot in origo.
	Vector2 offset = Vector2::zero();
	switch (shapeAsset->m_pivot)
	{
	default:
	case VectorShapeAsset::PtViewTopLeft:
		// Nothing to do.
		break;

	case VectorShapeAsset::PtViewCenter:
		offset = -viewBox.getCenter();
		break;

	case VectorShapeAsset::PtShapeCenter:
		offset = -bounds.getCenter();
		break;
	}
	
	// Set shape bounding box in output resource.
	outputShapeResource->m_bounds = Aabb2(
		bounds.mn + offset,
		bounds.mx + offset
	);

	// Define shape render vertex.
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, position)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, texCoord), 0));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, controlPoints), 1));

	Ref< render::Mesh > renderMesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		triangleCount * 3 * render::getVertexSize(vertexElements),
		render::ItUInt16,
		0
	);

	Vertex* vertex = static_cast< Vertex* >(renderMesh->getVertexBuffer()->lock());
	std::vector< render::Mesh::Part > meshParts;
	uint32_t permutateCount = 0;
	uint32_t vertexOffset = 0;

	for (std::list< std::pair< const SvgStyle*, TriangleProducer::Batch > >::const_iterator i = batches.begin(); i != batches.end(); ++i)
	{
		// Fill
		if (!i->second.trianglesFill.empty())
		{
			// Create vertices.
			for (uint32_t j = 0; j < i->second.trianglesFill.size(); ++j)
			{
				const Vector2& v = i->second.vertices[i->second.trianglesFill[j]];
				vertex->position[0] = v.x + offset.x;
				vertex->position[1] = v.y + offset.y;
				vertex->texCoord[0] = 0.0f;
				vertex->texCoord[1] = 0.0f;
				vertex->controlPoints[0] = c_controlPoints[j % 3][0];
				vertex->controlPoints[1] = c_controlPoints[j % 3][1];
				vertex++;
			}

			// Setup render mesh part.
			render::Mesh::Part part;
			part.primitives.setNonIndexed(
				render::PtTriangles,
				vertexOffset,
				uint32_t(i->second.trianglesFill.size() / 3)
			);
			meshParts.push_back(part);

			// Setup shape part.
			ShapeResource::Part shapePart;
			shapePart.fillColor = toColor4f(i->first);
			shapePart.curveSign = 0;
			outputShapeResource->m_parts.push_back(shapePart);

			// Increment vertex buffer offset.
			vertexOffset += i->second.trianglesFill.size();
		}

		// In
		if (!i->second.trianglesIn.empty())
		{
			// Create vertices.
			for (uint32_t j = 0; j < i->second.trianglesIn.size(); ++j)
			{
				const Vector2& v = i->second.vertices[i->second.trianglesIn[j]];
				vertex->position[0] = v.x + offset.x;
				vertex->position[1] = v.y + offset.y;
				vertex->texCoord[0] = 0.0f;
				vertex->texCoord[1] = 0.0f;
				vertex->controlPoints[0] = c_controlPoints[j % 3][0];
				vertex->controlPoints[1] = c_controlPoints[j % 3][1];
				vertex++;
			}

			// Setup render mesh part.
			render::Mesh::Part part;
			part.primitives.setNonIndexed(
				render::PtTriangles,
				vertexOffset,
				uint32_t(i->second.trianglesIn.size() / 3)
			);
			meshParts.push_back(part);

			// Setup shape part.
			ShapeResource::Part shapePart;
			shapePart.fillColor = toColor4f(i->first);
			shapePart.curveSign = 1;
			outputShapeResource->m_parts.push_back(shapePart);

			// Increment vertex buffer offset.
			vertexOffset += i->second.trianglesIn.size();
		}

		// Out
		if (!i->second.trianglesOut.empty())
		{
			// Create vertices.
			for (uint32_t j = 0; j < i->second.trianglesOut.size(); ++j)
			{
				const Vector2& v = i->second.vertices[i->second.trianglesOut[j]];
				vertex->position[0] = v.x + offset.x;
				vertex->position[1] = v.y + offset.y;
				vertex->texCoord[0] = 0.0f;
				vertex->texCoord[1] = 0.0f;
				vertex->controlPoints[0] = c_controlPoints[j % 3][0];
				vertex->controlPoints[1] = c_controlPoints[j % 3][1];
				vertex++;
			}

			// Setup render mesh part.
			render::Mesh::Part part;
			part.primitives.setNonIndexed(
				render::PtTriangles,
				vertexOffset,
				uint32_t(i->second.trianglesOut.size() / 3)
			);
			meshParts.push_back(part);

			// Setup shape part.
			ShapeResource::Part shapePart;
			shapePart.fillColor = toColor4f(i->first);
			shapePart.curveSign = -1;
			outputShapeResource->m_parts.push_back(shapePart);

			// Increment vertex buffer offset.
			vertexOffset += i->second.trianglesOut.size();
		}
	}
	renderMesh->getVertexBuffer()->unlock();

	renderMesh->setParts(meshParts);
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
