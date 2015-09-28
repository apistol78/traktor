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
#include "Render/Editor/Shader/ShaderGraphTechniques.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Spark/ShapeResource.h"
#include "Spark/Editor/ShapeAsset.h"
#include "Spark/Editor/ShapePipeline.h"
#include "Spark/Editor/ShapeShaderGenerator.h"
#include "Spark/Editor/Shape/Document.h"
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

const float c_pointScale = 100.0f;

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
	float controlPoints[2];
};
#pragma pack()

class TriangleProducer : public ShapeVisitor
{
public:
	struct Batch
	{
		AlignedVector< Vector2 > vertices;
		AlignedVector< uint32_t > trianglesFill;
		AlignedVector< uint32_t > trianglesIn;
		AlignedVector< uint32_t > trianglesOut;
	};

	TriangleProducer(float cubicApproximationError)
	:	m_cubicApproximationError(cubicApproximationError)
	,	m_viewBox(Vector2(0.0f, 0.0f), Vector2(100.0f, 100.0f))
	,	m_size(100.0f, 100.0f)
	{
		m_transformStack.push_back(Matrix33::identity());
	}

	virtual void enter(Shape* shape)
	{
		m_transformStack.push_back(m_transformStack.back() * shape->getTransform());
		const Matrix33& T = m_transformStack.back();

		Document* document = dynamic_type_cast< Document* >(shape);
		if (document)
		{
			m_viewBox = document->getViewBox();
			m_size = document->getSize();
		}

		if (shape->getStyle())
			m_styleStack.push_back(shape->getStyle());

		PathShape* pathShape = dynamic_type_cast< PathShape* >(shape);
		if (pathShape)
		{
			AlignedVector< Triangulator::Segment > segments;

			if (m_styleStack.empty() || !m_styleStack.back()->getFillEnable())
				return;

			const std::vector< SubPath >& subPaths = pathShape->getPath().getSubPaths();
			for (std::vector< SubPath >::const_iterator i = subPaths.begin(); i != subPaths.end(); ++i)
			{
				// Transform points into view.
				AlignedVector< Vector2 > points(i->points.size());
				for (uint32_t j = 0; j < i->points.size(); ++j)
					points[j] = m_size * (T * i->points[j] + m_viewBox.mn) / (m_viewBox.mx - m_viewBox.mn);

				// Transform origin into view.
				Vector2 origin = m_size * (T * i->origin + m_viewBox.mn) / (m_viewBox.mx - m_viewBox.mn);

				// Create triangulator segments.
				bool lastSubPath = bool(i == subPaths.end() - 1);
				switch (i->type)
				{
				case SptLinear:
					{
						for (uint32_t j = 0; j < points.size() - 1; ++j)
						{
							Triangulator::Segment s;
							s.curve = false;
							s.v[0] = Vector2i::fromVector2(points[j] * c_pointScale);
							s.v[1] = Vector2i::fromVector2(points[j + 1] * c_pointScale);
							segments.push_back(s);
						}
					}
					break;

				case SptQuadric:
					{
						for (uint32_t j = 0; j < points.size() - 1; j += 2)
						{
							Triangulator::Segment s;
							s.curve = true;
							s.v[0] = Vector2i::fromVector2(points[j] * c_pointScale);
							s.v[1] = Vector2i::fromVector2(points[j + 2] * c_pointScale);
							s.c = Vector2i::fromVector2(points[j + 1] * c_pointScale);
							segments.push_back(s);
						}
					}
					break;

				case SptCubic:
					{
						for (uint32_t j = 0; j < points.size() - 1; j += 3)
						{
							Bezier3rd b(
								points[j],
								points[j + 1],
								points[j + 2],
								points[j + 3]
							);

							AlignedVector< Bezier2nd > a;
							b.approximate(m_cubicApproximationError, a);

							for (AlignedVector< Bezier2nd >::const_iterator k = a.begin(); k != a.end(); ++k)
							{
								Triangulator::Segment s;
								s.curve = true;
								s.v[0] = Vector2i::fromVector2(k->cp0 * c_pointScale);
								s.v[1] = Vector2i::fromVector2(k->cp2 * c_pointScale);
								s.c = Vector2i::fromVector2(k->cp1 * c_pointScale);
								segments.push_back(s);
							}
						}
					}
					break;
				}

				if (lastSubPath)
				{
					Triangulator::Segment s;
					s.curve = false;
					s.v[0] = Vector2i::fromVector2(points.back() * c_pointScale);
					s.v[1] = Vector2i::fromVector2(origin * c_pointScale);
					segments.push_back(s);
				}
			}

			if (!segments.empty())
			{
				AlignedVector< Triangulator::Triangle > triangles;
				Triangulator().triangulate(segments, triangles);

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

						batch->vertices.push_back(i->v[0].toVector2() / c_pointScale);
						batch->vertices.push_back(i->v[1].toVector2() / c_pointScale);
						batch->vertices.push_back(i->v[2].toVector2() / c_pointScale);

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
	}

	virtual void leave(Shape* shape)
	{
		m_transformStack.pop_back();
		if (shape->getStyle())
			m_styleStack.pop_back();
	}

	const Aabb2& getViewBox() const { return m_viewBox; }

	const std::list< std::pair< const Style*, Batch > >& getBatches() const { return m_batches; }

private:
	float m_cubicApproximationError;
	std::list< std::pair< const Style*, Batch > > m_batches;
	Aabb2 m_viewBox;
	Vector2 m_size;
	RefArray< const Style > m_styleStack;
	AlignedVector< Matrix33 > m_transformStack;
	Matrix33 m_currentTransform;
};

bool setDefaultTechnique(render::ShaderGraph* shaderGraph)
{
	RefArray< render::VertexOutput > vertexOutputNodes;
	if (shaderGraph->findNodesOf< render::VertexOutput >(vertexOutputNodes) != 1)
		return false;

	for (RefArray< render::VertexOutput >::iterator i = vertexOutputNodes.begin(); i != vertexOutputNodes.end(); ++i)
		(*i)->setTechnique(L"Default");

	RefArray< render::PixelOutput > pixelOutputNodes;
	if (shaderGraph->findNodesOf< render::PixelOutput >(pixelOutputNodes) != 1)
		return false;

	for (RefArray< render::PixelOutput >::iterator i = pixelOutputNodes.begin(); i != pixelOutputNodes.end(); ++i)
		(*i)->setTechnique(L"Default");

	return true;
}

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
	ShapeShaderGenerator().addDependencies(pipelineDepends);
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
	TriangleProducer triangleProducer(shapeAsset->m_cubicApproximationError);
	shape->visit(&triangleProducer);

	// Create shape output resource.
	Ref< ShapeResource > outputShapeResource = new ShapeResource();

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
	const std::list< std::pair< const Style*, TriangleProducer::Batch > >& batches = triangleProducer.getBatches();

	// Count total number of triangles.
	uint32_t triangleCount = 0;
	for (std::list< std::pair< const Style*, TriangleProducer::Batch > >::const_iterator i = batches.begin(); i != batches.end(); ++i)
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
	for (std::list< std::pair< const Style*, TriangleProducer::Batch > >::const_iterator i = batches.begin(); i != batches.end(); ++i)
	{
		for (AlignedVector< Vector2 >::const_iterator j = i->second.vertices.begin(); j != i->second.vertices.end(); ++j)
		{
			T_FATAL_ASSERT(!isNanOrInfinite(j->x));
			T_FATAL_ASSERT(!isNanOrInfinite(j->y));
			bounds.contain(*j);
		}
	}

	// Determine offset to place pivot in origo.
	Vector2 offset = Vector2::zero();
	switch (shapeAsset->m_pivot)
	{
	default:
	case ShapeAsset::PtViewTopLeft:
		// Nothing to do.
		break;

	case ShapeAsset::PtViewCenter:
		offset = -viewBox.getCenter();
		break;

	case ShapeAsset::PtShapeCenter:
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
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, controlPoints)));

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

	for (std::list< std::pair< const Style*, TriangleProducer::Batch > >::const_iterator i = batches.begin(); i != batches.end(); ++i)
	{
		// Create master style shader.
		ShapeShaderGenerator shaderGenerator;
		Ref< render::ShaderGraph > masterStyleShader = shaderGenerator.generate(pipelineBuilder->getSourceDatabase(), i->first);
		if (!masterStyleShader)
		{
			log::error << L"Shape pipeline failed; unable to generate shader" << Endl;
			return false;
		}

		// Fill
		if (!i->second.trianglesFill.empty())
		{
			// Create "fill" shader.
			Ref< render::ShaderGraph > fillStyleShader = render::ShaderGraphTechniques(masterStyleShader).generate(L"Fill");
			setDefaultTechnique(fillStyleShader);

			// Build style shader.
			Guid outputShaderGuid = outputGuid.permutate(++permutateCount);
			std::wstring outputShaderPath = traktor::Path(outputPath).getPathOnly() + L"/" + outputGuid.format() + L"/" + outputShaderGuid.format();
			if (!pipelineBuilder->buildOutput(
				fillStyleShader,
				outputShaderPath,
				outputShaderGuid
			))
			{
				log::error << L"Shape pipeline failed; unable to build shader" << Endl;
				return false;
			}

			// Create vertices.
			for (uint32_t j = 0; j < i->second.trianglesFill.size(); ++j)
			{
				const Vector2& v = i->second.vertices[i->second.trianglesFill[j]];
				vertex->position[0] = v.x + offset.x;
				vertex->position[1] = v.y + offset.y;
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
			shapePart.shader = resource::Id< render::Shader >(outputShaderGuid);
			outputShapeResource->m_parts.push_back(shapePart);

			// Increment vertex buffer offset.
			vertexOffset += i->second.trianglesFill.size();
		}

		// In
		if (!i->second.trianglesIn.empty())
		{
			// Create "in" shader.
			Ref< render::ShaderGraph > inStyleShader = render::ShaderGraphTechniques(masterStyleShader).generate(L"In");
			setDefaultTechnique(inStyleShader);

			// Build style shader.
			Guid outputShaderGuid = outputGuid.permutate(++permutateCount);
			std::wstring outputShaderPath = traktor::Path(outputPath).getPathOnly() + L"/" + outputGuid.format() + L"/" + outputShaderGuid.format();
			if (!pipelineBuilder->buildOutput(
				inStyleShader,
				outputShaderPath,
				outputShaderGuid
			))
			{
				log::error << L"Shape pipeline failed; unable to build shader" << Endl;
				return false;
			}

			// Create vertices.
			for (uint32_t j = 0; j < i->second.trianglesIn.size(); ++j)
			{
				const Vector2& v = i->second.vertices[i->second.trianglesIn[j]];
				vertex->position[0] = v.x + offset.x;
				vertex->position[1] = v.y + offset.y;
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
			shapePart.shader = resource::Id< render::Shader >(outputShaderGuid);
			outputShapeResource->m_parts.push_back(shapePart);

			// Increment vertex buffer offset.
			vertexOffset += i->second.trianglesIn.size();
		}

		// Out
		if (!i->second.trianglesOut.empty())
		{
			// Create "out" shader.
			Ref< render::ShaderGraph > outStyleShader = render::ShaderGraphTechniques(masterStyleShader).generate(L"Out");
			setDefaultTechnique(outStyleShader);

			// Build style shader.
			Guid outputShaderGuid = outputGuid.permutate(++permutateCount);
			std::wstring outputShaderPath = traktor::Path(outputPath).getPathOnly() + L"/" + outputGuid.format() + L"/" + outputShaderGuid.format();
			if (!pipelineBuilder->buildOutput(
				outStyleShader,
				outputShaderPath,
				outputShaderGuid
			))
			{
				log::error << L"Shape pipeline failed; unable to build shader" << Endl;
				return false;
			}

			// Create vertices.
			for (uint32_t j = 0; j < i->second.trianglesOut.size(); ++j)
			{
				const Vector2& v = i->second.vertices[i->second.trianglesOut[j]];
				vertex->position[0] = v.x + offset.x;
				vertex->position[1] = v.y + offset.y;
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
			shapePart.shader = resource::Id< render::Shader >(outputShaderGuid);
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
