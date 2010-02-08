#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineReport.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshResource.h"
#include "Mesh/Editor/MaterialShaderGenerator.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshPipeline.h"
#include "Mesh/Editor/MeshUtilities.h"
#include "Mesh/Editor/Blend/BlendMeshConverter.h"
#include "Mesh/Editor/Indoor/IndoorMeshConverter.h"
#include "Mesh/Editor/Instance/InstanceMeshConverter.h"
#include "Mesh/Editor/Skinned/SkinnedMeshConverter.h"
#include "Mesh/Editor/Static/StaticMeshConverter.h"
#include "Mesh/Editor/Stream/StreamMeshConverter.h"
#include "Model/Model.h"
#include "Model/Formats/ModelFormat.h"
#include "Render/External.h"
#include "Render/FragmentLinker.h"
#include "Render/Nodes.h"
#include "Render/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

const static Guid c_guidVertexInterfaceGuid(L"{0A9BE5B4-4B45-B84A-AE16-57F6483436FC}");

class FragmentReaderAdapter : public render::FragmentLinker::FragmentReader
{
public:
	FragmentReaderAdapter(editor::IPipelineBuilder* pipelineBuilder)
	:	m_pipelineBuilder(pipelineBuilder)
	{
	}

	virtual Ref< const render::ShaderGraph > read(const Guid& fragmentGuid)
	{
		return m_pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(fragmentGuid);
	}

private:
	Ref< editor::IPipelineBuilder > m_pipelineBuilder;
};

Guid getVertexShaderGuid(MeshAsset::MeshType meshType)
{
	switch (meshType)
	{
	case MeshAsset::MtInstance:
		return Guid(L"{A714A83F-8442-6F48-A2A7-6EFA95EB75F3}");

	case MeshAsset::MtSkinned:
		return Guid(L"{69A3CF2E-9B63-0440-9410-70AB4AE127CE}");

	case MeshAsset::MtBlend:
		return Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}");

	case MeshAsset::MtIndoor:
		return Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}");

	case MeshAsset::MtStatic:
		return Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}");

	case MeshAsset::MtStream:
		return Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}");

	default:
		return Guid();
	}
}

Guid combineGuids(const Guid& g1, const Guid& g2)
{
	uint8_t d[16];
	for (int i = 0; i < 16; ++i)
		d[i] = g1[i] ^ g2[i];
	return Guid(d);
}

Guid incrementGuid(const Guid& g)
{
	uint8_t d[16];
	for (int i = 0; i < 16; ++i)
		d[i] = g[i];
	reinterpret_cast< uint32_t& >(d[12])++;
	return Guid(d);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshPipeline", 10, MeshPipeline, editor::IPipeline)

MeshPipeline::MeshPipeline()
:	m_promoteHalf(false)
{
}

bool MeshPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< editor::PropertyString >(L"Pipeline.AssetPath", L"");
	m_promoteHalf = settings->getProperty< editor::PropertyBoolean >(L"MeshPipeline.PromoteHalf", false);
	return true;
}

void MeshPipeline::destroy()
{
}

TypeInfoSet MeshPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MeshAsset >());
	return typeSet;
}

bool MeshPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const MeshAsset > asset = checked_type_cast< const MeshAsset* >(sourceAsset);
	T_ASSERT (asset);

	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, asset->getFileName());

	if (asset->getMeshType() != MeshAsset::MtStream)
		pipelineDepends->addDependency(fileName);
	else
	{
		// Stream meshes use many source models.
		RefArray< File > files;
		if (!FileSystem::getInstance().find(fileName, files))
		{
			log::error << L"Mesh pipeline failed; no models found" << Endl;
			return false;
		}

		for (RefArray< File >::const_iterator i = files.begin(); i != files.end(); ++i)
			pipelineDepends->addDependency((*i)->getPath());
	}

	// Determine vertex shader guid.
	Guid vertexShaderGuid = getVertexShaderGuid(asset->getMeshType());
	if (!vertexShaderGuid.isValid())
	{
		log::error << L"Mesh pipeline failed; unknown mesh asset type" << Endl;
		return false;
	}

	pipelineDepends->addDependency(vertexShaderGuid, editor::PdfUse);

	// Add dependencies to "fixed" material shaders.
	const std::map< std::wstring, Guid >& materialShaders = asset->getMaterialShaders();
	for (std::map< std::wstring, Guid >::const_iterator i = materialShaders.begin(); i != materialShaders.end(); ++i)
		pipelineDepends->addDependency(i->second, editor::PdfUse);

	return true;
}

bool MeshPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< const MeshAsset > asset = checked_type_cast< const MeshAsset* >(sourceAsset);
	const std::map< std::wstring, Guid >& materialShaders = asset->getMaterialShaders();
	std::map< std::wstring, model::Material > materials;
	RefArray< model::Model > models;

	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, asset->getFileName());
	
	// Locate source model(s).
	RefArray< File > files;
	if (!FileSystem::getInstance().find(fileName, files))
	{
		log::error << L"Mesh pipeline failed; unable to locate source model(s) (" << fileName.getPathName() << L")" << Endl;
		return false;
	}

	// Import source model(s); merge all materials into a single list (duplicates will be overridden).
	for (RefArray< File >::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		Path path = (*i)->getPath();

		log::info << L"Loading model \"" << path.getFileName() << L"\"..." << Endl;

		Ref< model::Model > model = model::ModelFormat::readAny(path);
		if (!model)
		{
			log::error << L"Mesh pipeline failed; unable to read source model (" << path.getPathName() << L")" << Endl;
			return false;
		}

		const std::vector< model::Material >& modelMaterials = model->getMaterials();
		if (model->getMaterials().empty())
		{
			log::error << L"Mesh pipeline failed; no materials in source model (" << path.getPathName() << L")" << Endl;
			return false;
		}

		for (std::vector< model::Material >::const_iterator j = modelMaterials.begin(); j != modelMaterials.end(); ++j)
			materials[j->getName()] = *j;

		models.push_back(model);
	}

	// Build materials.
	std::map< std::wstring, MeshConverter::MaterialInfo > materialInfo;
	std::vector< render::VertexElement > vertexElements;
	uint32_t vertexElementOffset = 0;

	Guid vertexShaderGuid = getVertexShaderGuid(asset->getMeshType());
	T_ASSERT (vertexShaderGuid.isValid());

	Guid materialGuid = combineGuids(vertexShaderGuid, outputGuid);
	T_ASSERT (materialGuid.isValid());

	MaterialShaderGenerator generator(
		pipelineBuilder->getSourceDatabase()
	);

	FragmentReaderAdapter fragmentReader(pipelineBuilder);

	for (std::map< std::wstring, model::Material >::const_iterator i = materials.begin(); i != materials.end(); ++i)
	{
		Ref< const render::ShaderGraph > materialShaderGraph;

		std::map< std::wstring, Guid >::const_iterator it = materialShaders.find(i->first);
		if (it != materialShaders.end())
		{
			materialShaderGraph = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(it->second);
			if (!materialShaderGraph)
			{
				log::error << L"Mesh pipeline failed; unable to read material shader \"" << i->first << L"\"" << Endl;
				return false;
			}
		}
		else
		{
			materialShaderGraph = generator.generate(i->second);
			if (!materialShaderGraph)
			{
				log::error << L"Mesh pipeline failed; unable to generate material shader \"" << i->first << L"\"" << Endl;
				return false;
			}
		}

		// Replace place-holder vertex fragments with actual implementation.
		RefArray< render::External > externalNodes;
		materialShaderGraph->findNodesOf< render::External >(externalNodes);
		for (RefArray< render::External >::iterator j = externalNodes.begin(); j != externalNodes.end(); ++j)
		{
			if ((*j)->getFragmentGuid() == c_guidVertexInterfaceGuid)
				(*j)->setFragmentGuid(vertexShaderGuid);
		}

		// Link shader fragments.
		materialShaderGraph = render::FragmentLinker(fragmentReader).resolve(materialShaderGraph, true);
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to link shader fragments, material shader \"" << i->first << L"\"" << Endl;
			return false;
		}

		// Remove unused branches from shader graph.
		materialShaderGraph = render::ShaderGraphOptimizer(materialShaderGraph).removeUnusedBranches();
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to remove unused branches, material shader \"" << i->first << L"\"" << Endl;
			return false;
		}

		// Build vertex declaration from shader vertex inputs.
		RefArray< render::VertexInput > vertexInputNodes;
		materialShaderGraph->findNodesOf< render::VertexInput >(vertexInputNodes);
		for (RefArray< render::VertexInput >::iterator j = vertexInputNodes.begin(); j != vertexInputNodes.end(); ++j)
		{
			bool elementDeclared = false;

			render::DataType elementDataType = (*j)->getDataType();
			if (m_promoteHalf)
			{
				if (elementDataType == render::DtHalf2)
					elementDataType = render::DtFloat2;
				else if (elementDataType == render::DtHalf4)
					elementDataType = render::DtFloat4;
			}

			// Is it already added to vertex declaration?
			for (std::vector< render::VertexElement >::iterator k = vertexElements.begin(); k != vertexElements.end(); ++k)
			{
				if (
					(*j)->getDataUsage() == k->getDataUsage() &&
					(*j)->getIndex() == k->getIndex()
				)
				{
					if (elementDataType != k->getDataType())
						log::warning << L"Identical vertex input usage but different types (" << render::getDataTypeName(elementDataType) << L" and " << render::getDataTypeName(k->getDataType()) << L")" << Endl;
					elementDeclared = true;
					break;
				}
			}

			if (!elementDeclared)
			{
				render::VertexElement element(
					(*j)->getDataUsage(),
					elementDataType,
					vertexElementOffset,
					(*j)->getIndex()
				);
				vertexElements.push_back(element);
				vertexElementOffset += element.getSize();
			}
		}

		// Build material shader.
		std::wstring materialPath = Path(outputPath).getPathOnly() + L"/" + outputGuid.format() + L"/" + i->first;
		if (!pipelineBuilder->buildOutput(
			materialShaderGraph,
			0,
			i->first,
			materialPath,
			materialGuid
		))
		{
			log::error << L"Mesh pipeline failed; unable to build material \"" << i->first << L"\"" << Endl;
			return false;
		}

		// Insert into material map.
		MeshConverter::MaterialInfo mi = { materialGuid, isOpaqueMaterial(materialShaderGraph) };
		materialInfo.insert(std::make_pair(i->first, mi));

		// Increment guid for each material, quite hackish but won't guid;s still be universally unique?
		materialGuid = incrementGuid(materialGuid);
	}

	// Create mesh converter.
	Ref< MeshConverter > converter;
	switch (asset->getMeshType())
	{
	case MeshAsset::MtBlend:
		converter = new BlendMeshConverter();
		break;
	case MeshAsset::MtIndoor:
		converter = new IndoorMeshConverter();
		break;
	case MeshAsset::MtInstance:
		converter = new InstanceMeshConverter();
		break;
	case MeshAsset::MtSkinned:
		converter = new SkinnedMeshConverter();
		break;
	case MeshAsset::MtStatic:
		converter = new StaticMeshConverter();
		break;
	case MeshAsset::MtStream:
		converter = new StreamMeshConverter();
		break;
	default:
		log::error << L"Mesh pipeline failed; unknown mesh asset type" << Endl;
		return false;
	}

	// Create render mesh.
	Ref< MeshResource > resource = converter->createResource();
	if (!resource)
	{
		log::error << L"Mesh pipeline failed; unable to create mesh resource" << Endl;
		return false;
	}

	// Create output instance.
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!outputInstance)
	{
		log::error << L"Mesh pipeline failed; unable to create output instance" << Endl;
		return false;
	}

	// Open asset data stream.
	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Mesh pipeline failed; unable to create mesh data stream" << Endl;
		outputInstance->revert();
		return false;
	}

	// Convert mesh asset.
	if (!converter->convert(
		models,
		materialInfo,
		vertexElements,
		resource,
		stream
	))
	{
		log::error << L"Mesh pipeline failed; unable to convert mesh" << Endl;
		return false;
	}

	stream->close();

	// Commit resource.
	outputInstance->setObject(resource);
	if (!outputInstance->commit())
	{
		log::error << L"Mesh pipeline failed; unable to commit output instance" << Endl;
		return false;
	}

	// Create report.
	Ref< editor::IPipelineReport > report = pipelineBuilder->createReport(L"Mesh", outputGuid);
	if (report)
		report->set(L"type", int32_t(asset->getMeshType()));

	return true;
}

	}
}
