#include "Mesh/Editor/MeshPipeline.h"
#include "Mesh/Editor/MeshPipelineParams.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/Blend/BlendMeshConverter.h"
#include "Mesh/Editor/Indoor/IndoorMeshConverter.h"
#include "Mesh/Editor/Instance/InstanceMeshConverter.h"
#include "Mesh/Editor/Skinned/SkinnedMeshConverter.h"
#include "Mesh/Editor/Static/StaticMeshConverter.h"
#include "Mesh/MeshResource.h"
#include "Editor/PipelineManager.h"
#include "Editor/Settings.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Model/Formats/ModelFormat.h"
#include "Model/Model.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/ShaderGraph.h"
#include "Render/External.h"
#include "Render/Nodes.h"
#include "Render/FragmentLinker.h"
#include "Core/Io/Stream.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

class FragmentReaderAdapter : public render::FragmentLinker::FragmentReader
{
public:
	FragmentReaderAdapter(editor::PipelineManager* pipelineManager)
	:	m_pipelineManager(pipelineManager)
	{
	}

	virtual const render::ShaderGraph* read(const Guid& fragmentGuid)
	{
		return m_pipelineManager->getObjectReadOnly< render::ShaderGraph >(fragmentGuid);
	}

private:
	Ref< editor::PipelineManager > m_pipelineManager;
};

Guid combineGuids(const Guid& g1, const Guid& g2)
{
	uint8_t d[16];
	for (int i = 0; i < 16; ++i)
		d[i] = g1[i] ^ g2[i];
	return Guid(d);
}

		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.mesh.MeshPipeline", MeshPipeline, editor::IPipeline)

bool MeshPipeline::create(const editor::Settings* settings)
{
	m_materialSourcePath = settings->getProperty< editor::PropertyString >(L"MeshPipeline.MaterialSourcePath");
	m_materialOutputPath = settings->getProperty< editor::PropertyString >(L"MeshPipeline.MaterialOutputPath");
	m_defaultMaterial = settings->getProperty< editor::PropertyGuid >(L"MeshPipeline.DefaultMaterial");
	return true;
}

void MeshPipeline::destroy()
{
}

uint32_t MeshPipeline::getVersion() const
{
	return 3;
}

TypeSet MeshPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< MeshAsset >());
	return typeSet;
}

bool MeshPipeline::buildDependencies(
	editor::PipelineManager* pipelineManager,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const MeshAsset > asset = checked_type_cast< const MeshAsset* >(sourceAsset);
	T_ASSERT (asset);

	// Create parameter object.
	Ref< MeshPipelineParams > params = gc_new< MeshPipelineParams >();

	// Import source model.
	Path fileName = asset->getFileName();
	params->m_model = model::ModelFormat::readAny(fileName);
	if (!params->m_model)
	{
		log::error << L"Mesh pipeline failed; unable to read source model (" << fileName.getPathName() << L")" << Endl;
		return false;
	}

	// Determine vertex shader guid.
	Guid vertexShaderGuid;
	switch (asset->m_meshType)
	{
	case MeshAsset::MtInstance:
		vertexShaderGuid = Guid(L"{A714A83F-8442-6F48-A2A7-6EFA95EB75F3}");
		break;
	case MeshAsset::MtSkinned:
		vertexShaderGuid = Guid(L"{69A3CF2E-9B63-0440-9410-70AB4AE127CE}");
		break;
	case MeshAsset::MtBlend:
		vertexShaderGuid = Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}");
		break;
	case MeshAsset::MtIndoor:
		vertexShaderGuid = Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}");
		break;
	case MeshAsset::MtStatic:
		vertexShaderGuid = Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}");
		break;
	default:
		log::error << L"Mesh pipeline failed; unknown mesh asset type" << Endl;
		return false;
	}

	// Create material asset dependencies.
	const std::vector< model::Material >& materials = params->m_model->getMaterials();
	for (std::vector< model::Material >::const_iterator i = materials.begin(); i != materials.end(); ++i)
	{
		const std::wstring& name = i->getName();

		// Find material shader; first try in same group as the source instance.
		Ref< db::Instance > materialShaderInstance;
		materialShaderInstance = pipelineManager->getSourceDatabase()->getInstance(
			sourceInstance->getParent()->getPath() + L"/" + name,
			&type_of< render::ShaderGraph >()
		);
		if (!materialShaderInstance)
		{
			materialShaderInstance = pipelineManager->getSourceDatabase()->getInstance(
				m_materialSourcePath + L"/" + name,
				&type_of< render::ShaderGraph >()
			);
		}
		if (!materialShaderInstance)
		{
			log::warning << L"No material shader \"" << name << L"\" found; using default material" << Endl;
			materialShaderInstance = pipelineManager->getSourceDatabase()->getInstance(m_defaultMaterial);
			T_ASSERT (materialShaderInstance);
		}

		// Create a new guid from vertex+material guids; must be generated the
		// same every time since it's being used by pipeline manager to track
		// dependencies.
		Guid materialGuid = combineGuids(vertexShaderGuid, materialShaderInstance->getGuid());
		T_ASSERT (materialGuid.isValid());

		// Add no-build dependencies to shader fragments.
		pipelineManager->addDependency(vertexShaderGuid, false);
		pipelineManager->addDependency(materialShaderInstance->getGuid(), false);

		// Generate combined shader; @fixme the combined graph is possibly created multiple times.
		Ref< const render::ShaderGraph > materialShaderGraph = materialShaderInstance->getObject< render::ShaderGraph >();
		if (!materialShaderGraph)
			continue;

		RefArray< render::External > externalNodes;
		materialShaderGraph->findNodesOf< render::External >(externalNodes);

		// Replace vertex shader placeholder with actual implementation.
		const Guid c_guidVertexInterfaceGuid(L"{0A9BE5B4-4B45-B84A-AE16-57F6483436FC}");
		for (RefArray< render::External >::iterator i = externalNodes.begin(); i != externalNodes.end(); ++i)
		{
			if ((*i)->getFragmentGuid() == c_guidVertexInterfaceGuid)
				(*i)->setFragmentGuid(vertexShaderGuid);
		}

		pipelineManager->addDependency(
			materialShaderGraph,
			name,
			m_materialOutputPath + L"/" + vertexShaderGuid.format() + L"/" + materialShaderInstance->getGuid().format(),
			materialGuid
		);

		// Remember material information.
		MeshPipelineParams::MaterialInfo materialInfo;
		materialInfo.guid = materialGuid;
		materialInfo.graph = materialShaderGraph;
		params->m_materialMap.insert(std::make_pair(
			name,
			materialInfo
		));
	}

	// Return build parameter object.
	outBuildParams = params;
	return true;
}

bool MeshPipeline::buildOutput(
	editor::PipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< const MeshAsset > asset = checked_type_cast< const MeshAsset* >(sourceAsset);
	Ref< const MeshPipelineParams > params = checked_type_cast< const MeshPipelineParams* >(buildParams);

	// Investigate which vertex elements are being used by material; need a fully qualitifed graph.
	FragmentReaderAdapter fragmentReader(pipelineManager);
	std::vector< render::VertexElement > vertexElements;
	uint32_t vertexElementOffset = 0;

	for (std::map< std::wstring, MeshPipelineParams::MaterialInfo >::const_iterator i = params->m_materialMap.begin(); i != params->m_materialMap.end(); ++i)
	{
		// Link shader fragments.
		Ref< render::ShaderGraph > materialGraph = render::FragmentLinker(fragmentReader).resolve(i->second.graph);
		if (!materialGraph)
		{
			log::error << L"MeshPipeline failed; unable to link shader fragments" << Endl;
			return false;
		}

		// Remove unused branches from shader graph.
		materialGraph = render::ShaderGraphOptimizer(materialGraph).removeUnusedBranches();
		if (!materialGraph)
		{
			log::error << L"MeshPipeline failed; unable to remove unused branches" << Endl;
			return false;
		}

		RefArray< render::VertexInput > vertexInputNodes;
		materialGraph->findNodesOf< render::VertexInput >(vertexInputNodes);
		for (RefArray< render::VertexInput >::iterator j = vertexInputNodes.begin(); j != vertexInputNodes.end(); ++j)
		{
			bool elementDeclared = false;

			// Is it already added to vertex declaration?
			for (std::vector< render::VertexElement >::iterator k = vertexElements.begin(); k != vertexElements.end(); ++k)
			{
				if (
					(*j)->getDataUsage() == k->getDataUsage() &&
					(*j)->getIndex() == k->getIndex()
				)
				{
					if ((*j)->getDataType() != k->getDataType())
						log::warning << L"Identical vertex input usage but different types (" << render::getDataTypeName((*j)->getDataType()) << L" and " << render::getDataTypeName(k->getDataType()) << L")" << Endl;
					elementDeclared = true;
					break;
				}
			}

			if (!elementDeclared)
			{
				render::VertexElement element(
					(*j)->getDataUsage(),
					(*j)->getDataType(),
					vertexElementOffset,
					(*j)->getIndex()
				);
				vertexElements.push_back(element);
				vertexElementOffset += element.getSize();
			}
		}
	}

	// Create mesh converter.
	Ref< MeshConverter > converter;
	switch (asset->m_meshType)
	{
	case MeshAsset::MtBlend:
		converter = gc_new< BlendMeshConverter >();
		break;
	case MeshAsset::MtIndoor:
		converter = gc_new< IndoorMeshConverter >();
		break;
	case MeshAsset::MtInstance:
		converter = gc_new< InstanceMeshConverter >();
		break;
	case MeshAsset::MtSkinned:
		converter = gc_new< SkinnedMeshConverter >();
		break;
	case MeshAsset::MtStatic:
		converter = gc_new< StaticMeshConverter >();
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
	Ref< db::Instance > outputInstance = pipelineManager->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!outputInstance)
	{
		log::error << L"Mesh pipeline failed; unable to create output instance" << Endl;
		return false;
	}

	// Open asset data stream.
	Ref< Stream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Mesh pipeline failed; unable to create mesh data stream" << Endl;
		outputInstance->revert();
		return false;
	}

	// Convert mesh asset.
	if (!converter->convert(
		*params->m_model,
		params->m_materialMap,
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
	outputInstance->commit();

	return true;
}

	}
}
