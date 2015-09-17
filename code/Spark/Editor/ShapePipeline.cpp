#pragma optimize( "", off )

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
#include "Render/Shader/ShaderGraph.h"
#include "Spark/ShapeResource.h"
#include "Spark/Editor/ShapeAsset.h"
#include "Spark/Editor/ShapePipeline.h"
#include "Spark/Editor/ShapeShaderGenerator.h"
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
	float color[4];
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
	{
		m_transformStack.push_back(Matrix33::identity());
	}

	virtual void enter(Shape* shape)
	{
		m_transformStack.push_back(m_transformStack.back() * shape->getTransform());

		const Matrix33& T = m_transformStack.back();

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
						//log::info << L"SptLinear; " << int32_t(i->points.size()) << L" point(s), (" << (i->closed ? L"closed" : L"open") << L")" << Endl;
						for (uint32_t j = 0; j < i->points.size() - 1; ++j)
						{
							Triangulator::Segment s;
							s.curve = false;
							s.v[0] = Vector2i::fromVector2((T * i->points[j]) * c_pointScale);
							s.v[1] = Vector2i::fromVector2((T * i->points[j + 1]) * c_pointScale);
							segments.push_back(s);
						}
						if (i->closed)
						{
							Triangulator::Segment s;
							s.curve = false;
							s.v[0] = Vector2i::fromVector2((T * i->points.back()) * c_pointScale);
							s.v[1] = Vector2i::fromVector2((T * i->points.front()) * c_pointScale);
							segments.push_back(s);
						}
					}
					break;

				case SptQuadric:
					{
						//log::info << L"SptQuadric; " << int32_t(i->points.size()) << L" point(s), (" << (i->closed ? L"closed" : L"open") << L")" << Endl;
						for (uint32_t j = 0; j < i->points.size() - 1; j += 2)
						{
							Triangulator::Segment s;
							s.curve = true;
							s.v[0] = Vector2i::fromVector2((T * i->points[j]) * c_pointScale);
							s.v[1] = Vector2i::fromVector2((T * i->points[j + 2]) * c_pointScale);
							s.c = Vector2i::fromVector2((T * i->points[j + 1]) * c_pointScale);
							segments.push_back(s);
						}
						//if (i->closed)
						//{
						//	Triangulator::Segment s;
						//	s.curve = false;
						//	s.v[0] = Vector2i::fromVector2((T * i->points.back()) * c_pointScale);
						//	s.v[1] = Vector2i::fromVector2((T * i->points.front()) * c_pointScale);
						//	segments.push_back(s);
						//}
					}
					break;

				case SptCubic:
					{
						//log::info << L"SptCubic; " << int32_t(i->points.size()) << L" point(s), (" << (i->closed ? L"closed" : L"open") << L")" << Endl;
						for (uint32_t j = 0; j < i->points.size() - 1; j += 3)
						{
							Bezier3rd b(
								T * i->points[j],
								T * i->points[j + 1],
								T * i->points[j + 2],
								T * i->points[j + 3]
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
						//if (i->closed)
						//{
						//	Triangulator::Segment s;
						//	s.curve = false;
						//	s.v[0] = Vector2i::fromVector2((T * i->points.back()) * c_pointScale);
						//	s.v[1] = Vector2i::fromVector2((T * i->points.front()) * c_pointScale);
						//	segments.push_back(s);
						//}
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

						batch.vertices.push_back(i->v[0].toVector2() / c_pointScale);
						batch.vertices.push_back(i->v[1].toVector2() / c_pointScale);
						batch.vertices.push_back(i->v[2].toVector2() / c_pointScale);

						if (i->type == Triangulator::TcFill)
						{
							batch.trianglesFill.push_back(indexBase + 0);
							batch.trianglesFill.push_back(indexBase + 1);
							batch.trianglesFill.push_back(indexBase + 2);
						}
						else if (i->type == Triangulator::TcIn)
						{
							batch.trianglesIn.push_back(indexBase + 0);
							batch.trianglesIn.push_back(indexBase + 1);
							batch.trianglesIn.push_back(indexBase + 2);
						}
						else if (i->type == Triangulator::TcOut)
						{
							batch.trianglesOut.push_back(indexBase + 0);
							batch.trianglesOut.push_back(indexBase + 1);
							batch.trianglesOut.push_back(indexBase + 2);
						}
					}

					m_batches.push_back(std::make_pair(m_style, batch));
				}
			}
		}
	}

	virtual void leave(Shape* shape)
	{
		m_transformStack.pop_back();
	}

	const std::list< std::pair< const Style*, Batch > >& getBatches() const { return m_batches; }

private:
	float m_cubicApproximationError;
	std::list< std::pair< const Style*, Batch > > m_batches;
	Ref< const Style > m_style;

	AlignedVector< Matrix33 > m_transformStack;
	Matrix33 m_currentTransform;
};

Guid incrementGuid(const Guid& g)
{
	uint8_t d[16];
	for (int i = 0; i < 16; ++i)
		d[i] = g[i];
	reinterpret_cast< uint32_t& >(d[12])++;
	return Guid(d);
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
	pipelineDepends->addDependency(shapeAsset->m_shader, editor::PdfUse);
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

	// Create shape shader.
	ShapeShaderGenerator shaderGenerator;
	Ref< render::ShaderGraph > outputShapeShader = shaderGenerator.generate(pipelineBuilder->getSourceDatabase(), shapeAsset->m_shader);
	if (!outputShapeShader)
	{
		log::error << L"Shape pipeline failed; unable to generate shader" << Endl;
		return false;
	}

	// Build shape shader.
	Guid outputShaderGuid = incrementGuid(outputGuid);
	std::wstring outputShaderPath = traktor::Path(outputPath).getPathOnly() + L"/" + outputGuid.format() + L"/Shader";
	if (!pipelineBuilder->buildOutput(
		outputShapeShader,
		outputShaderPath,
		outputShaderGuid
	))
	{
		log::error << L"Shape pipeline failed; unable to build shader" << Endl;
		return false;
	}

	// Create shape output resource.
	Ref< ShapeResource > outputShapeResource = new ShapeResource();
	outputShapeResource->m_shader = resource::Id< render::Shader >(outputShaderGuid);

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

	// Measure shape bounds.
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
	outputShapeResource->m_bounds = bounds;

	// Define shape render vertex.
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, position)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, controlPoints)));
	vertexElements.push_back(render::VertexElement(render::DuColor, render::DtFloat4, offsetof(Vertex, color)));

	Ref< render::Mesh > renderMesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		triangleCount * 3 * render::getVertexSize(vertexElements),
		render::ItUInt16,
		0
	);

	Vertex* vertex = static_cast< Vertex* >(renderMesh->getVertexBuffer()->lock());
	uint32_t vertexOffset = 0;

	std::vector< render::Mesh::Part > meshParts;

	for (std::list< std::pair< const Style*, TriangleProducer::Batch > >::const_iterator i = batches.begin(); i != batches.end(); ++i)
	{
		// Fill
		{
			for (uint32_t j = 0; j < i->second.trianglesFill.size(); ++j)
			{
				const Vector2& v = i->second.vertices[i->second.trianglesFill[j]];

				if (shapeAsset->m_center)
				{
					vertex->position[0] = v.x - bounds.getCenter().x;
					vertex->position[1] = v.y - bounds.getCenter().y;
				}
				else
				{
					vertex->position[0] = v.x;
					vertex->position[1] = v.y;
				}

				vertex->controlPoints[0] = c_controlPoints[j % 3][0];
				vertex->controlPoints[1] = c_controlPoints[j % 3][1];
				vertex->color[0] = i->first->getFill().r / 255.0f;
				vertex->color[1] = i->first->getFill().g / 255.0f;
				vertex->color[2] = i->first->getFill().b / 255.0f;
				vertex->color[3] = i->first->getFill().a / 255.0f;

				vertex++;
			}

			render::Mesh::Part part;
			part.primitives.setNonIndexed(
				render::PtTriangles,
				vertexOffset,
				uint32_t(i->second.trianglesFill.size() / 3)
			);
			meshParts.push_back(part);
			outputShapeResource->m_parts.push_back(0);

			vertexOffset += i->second.trianglesFill.size();
		}

		// In
		{
			for (uint32_t j = 0; j < i->second.trianglesIn.size(); ++j)
			{
				const Vector2& v = i->second.vertices[i->second.trianglesIn[j]];

				if (shapeAsset->m_center)
				{
					vertex->position[0] = v.x - bounds.getCenter().x;
					vertex->position[1] = v.y - bounds.getCenter().y;
				}
				else
				{
					vertex->position[0] = v.x;
					vertex->position[1] = v.y;
				}

				vertex->controlPoints[0] = c_controlPoints[j % 3][0];
				vertex->controlPoints[1] = c_controlPoints[j % 3][1];
				vertex->color[0] = i->first->getFill().r / 255.0f;
				vertex->color[1] = i->first->getFill().g / 255.0f;
				vertex->color[2] = i->first->getFill().b / 255.0f;
				vertex->color[3] = i->first->getFill().a / 255.0f;

				vertex++;
			}

			render::Mesh::Part part;
			part.primitives.setNonIndexed(
				render::PtTriangles,
				vertexOffset,
				uint32_t(i->second.trianglesIn.size() / 3)
			);
			meshParts.push_back(part);
			outputShapeResource->m_parts.push_back(1);

			vertexOffset += i->second.trianglesIn.size();
		}

		// Out
		{
			for (uint32_t j = 0; j < i->second.trianglesOut.size(); ++j)
			{
				const Vector2& v = i->second.vertices[i->second.trianglesOut[j]];

				if (shapeAsset->m_center)
				{
					vertex->position[0] = v.x - bounds.getCenter().x;
					vertex->position[1] = v.y - bounds.getCenter().y;
				}
				else
				{
					vertex->position[0] = v.x;
					vertex->position[1] = v.y;
				}

				vertex->controlPoints[0] = c_controlPoints[j % 3][0];
				vertex->controlPoints[1] = c_controlPoints[j % 3][1];
				vertex->color[0] = i->first->getFill().r / 255.0f;
				vertex->color[1] = i->first->getFill().g / 255.0f;
				vertex->color[2] = i->first->getFill().b / 255.0f;
				vertex->color[3] = i->first->getFill().a / 255.0f;

				vertex++;
			}

			render::Mesh::Part part;
			part.primitives.setNonIndexed(
				render::PtTriangles,
				vertexOffset,
				uint32_t(i->second.trianglesOut.size() / 3)
			);
			meshParts.push_back(part);
			outputShapeResource->m_parts.push_back(2);

			vertexOffset += i->second.trianglesOut.size();
		}
	}
	renderMesh->getVertexBuffer()->unlock();

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
