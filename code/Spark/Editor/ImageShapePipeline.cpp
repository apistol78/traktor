#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Render/VertexBuffer.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"
#include "Render/Mesh/SystemMeshFactory.h"
#include "Spark/ShapeResource.h"
#include "Spark/Editor/ImageShapeAsset.h"
#include "Spark/Editor/ImageShapePipeline.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

#pragma pack(1)
struct Vertex
{
	float position[2];
	float texCoord[2];
};
#pragma pack()

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.ImageShapePipeline", 0, ImageShapePipeline, editor::IPipeline)

ImageShapePipeline::ImageShapePipeline()
{
}

bool ImageShapePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void ImageShapePipeline::destroy()
{
}

TypeInfoSet ImageShapePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ImageShapeAsset >());
	return typeSet;
}

bool ImageShapePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const ImageShapeAsset* shapeAsset = checked_type_cast< const ImageShapeAsset* >(sourceAsset);
	pipelineDepends->addDependency(traktor::Path(m_assetPath), shapeAsset->getFileName().getOriginal());
	pipelineDepends->addDependency(Guid(L"{E411A034-2FDA-4B44-A378-700D1CB8B6E4}"), editor::PdfBuild | editor::PdfResource);
	return true;
}

bool ImageShapePipeline::buildOutput(
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
	const ImageShapeAsset* shapeAsset = checked_type_cast< const ImageShapeAsset* >(sourceAsset);

	// Open stream to source file.
	Ref< IStream > sourceStream = pipelineBuilder->openFile(traktor::Path(m_assetPath), shapeAsset->getFileName().getOriginal());
	if (!sourceStream)
	{
		log::error << L"Image shape pipeline failed; unable to open file \"" << shapeAsset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	// Read image from stream.
	Ref< drawing::Image > image = drawing::Image::load(sourceStream, shapeAsset->getFileName().getExtension());
	if (!image)
	{
		log::error << L"Image shape pipeline failed; unable to read file \"" << shapeAsset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	sourceStream->close();
	sourceStream = 0;

	// Create texture from image.
	Ref< render::TextureOutput > imageOutput = new render::TextureOutput();
	imageOutput->m_textureFormat = render::TfR8G8B8A8;
	imageOutput->m_generateNormalMap = false;
	imageOutput->m_scaleDepth = 0.0f;
	imageOutput->m_generateMips = true;
	imageOutput->m_keepZeroAlpha = false;
	imageOutput->m_textureType = render::Tt2D;
	imageOutput->m_hasAlpha = false;
	imageOutput->m_ignoreAlpha = false;
	imageOutput->m_scaleImage = false;
	imageOutput->m_scaleWidth = 0;
	imageOutput->m_scaleHeight = 0;
	imageOutput->m_enableCompression = false;
	imageOutput->m_enableNormalMapCompression = false;
	imageOutput->m_inverseNormalMapY = false;
	imageOutput->m_linearGamma = true;
	imageOutput->m_generateSphereMap = false;
	imageOutput->m_preserveAlphaCoverage = false;
	imageOutput->m_alphaCoverageReference = 0.0f;
	imageOutput->m_sharpenRadius = 0;
	imageOutput->m_sharpenStrength = 0.0f;
	imageOutput->m_systemTexture = true;

	Guid imageOutputGuid = outputGuid.permutate(1);
	std::wstring imageOutputPath = traktor::Path(outputPath).getPathOnly() + L"/" + outputGuid.format() + L"/Texture";

	if (!pipelineBuilder->buildOutput(
		imageOutput,
		imageOutputPath,
		imageOutputGuid,
		image
	))
	{
		log::error << L"Image shape pipeline failed; unable to build texture" << Endl;
		return false;
	}

	float w = float(image->getWidth());
	float h = float(image->getHeight());

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
		log::error << L"Image shape pipeline failed; unable to create output instance" << Endl;
		return false;
	}

	// Open asset data stream.
	Ref< IStream > dataStream = outputInstance->writeData(L"Data");
	if (!dataStream)
	{
		log::error << L"Image shape pipeline failed; unable to create mesh data stream" << Endl;
		outputInstance->revert();
		return false;
	}

	// Measure initial shape bounds.
	Aabb2 bounds(
		Vector2(0.0f, 0.0f),
		Vector2(w, h)
	);

	// Determine offset to place pivot in origo.
	Vector2 offset = Vector2::zero();
	switch (shapeAsset->m_pivot)
	{
	default:
	case ImageShapeAsset::PtTopLeft:
		// Nothing to do.
		break;

	case ImageShapeAsset::PtCenter:
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
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, texCoord)));

	Ref< render::Mesh > renderMesh = render::SystemMeshFactory().createMesh(
		vertexElements,
		2 * 3 * render::getVertexSize(vertexElements),
		render::ItUInt16,
		0
	);

	Vertex* vertex = static_cast< Vertex* >(renderMesh->getVertexBuffer()->lock());
	vertex->position[0] = offset.x    ; vertex->position[1] = offset.y    ; vertex->texCoord[0] = 0.0f; vertex->texCoord[1] = 0.0f; ++vertex;
	vertex->position[0] = offset.x + w; vertex->position[1] = offset.y    ; vertex->texCoord[0] = 1.0f; vertex->texCoord[1] = 0.0f; ++vertex;
	vertex->position[0] = offset.x    ; vertex->position[1] = offset.y + h; vertex->texCoord[0] = 0.0f; vertex->texCoord[1] = 1.0f; ++vertex;
	vertex->position[0] = offset.x + w; vertex->position[1] = offset.y    ; vertex->texCoord[0] = 1.0f; vertex->texCoord[1] = 0.0f; ++vertex;
	vertex->position[0] = offset.x + w; vertex->position[1] = offset.y + h; vertex->texCoord[0] = 1.0f; vertex->texCoord[1] = 1.0f; ++vertex;
	vertex->position[0] = offset.x    ; vertex->position[1] = offset.y + h; vertex->texCoord[0] = 0.0f; vertex->texCoord[1] = 1.0f; ++vertex;
	renderMesh->getVertexBuffer()->unlock();

	// Setup render mesh part.
	std::vector< render::Mesh::Part > meshParts;
	render::Mesh::Part part;
	part.primitives.setNonIndexed(render::PtTriangles, 0, 2);
	meshParts.push_back(part);

	// Setup shape part.
	ShapeResource::Part shapePart;
	shapePart.texture = resource::Id< render::ITexture >(imageOutputGuid);
	shapePart.fillColor = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	shapePart.curveSign = 0;
	outputShapeResource->m_parts.push_back(shapePart);

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

Ref< ISerializable > ImageShapePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
