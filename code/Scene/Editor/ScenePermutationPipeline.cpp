#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/ScenePermutationAsset.h"
#include "Scene/Editor/ScenePermutationPipeline.h"
#include "World/WorldRenderSettings.h"
#include "World/Editor/LayerEntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.ScenePermutationPipeline", 3, ScenePermutationPipeline, editor::IPipeline)

bool ScenePermutationPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void ScenePermutationPipeline::destroy()
{
}

TypeInfoSet ScenePermutationPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ScenePermutationAsset >());
	return typeSet;
}

bool ScenePermutationPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const ScenePermutationAsset* scenePermutationAsset = checked_type_cast< const ScenePermutationAsset*, false >(sourceAsset);

	Ref< const SceneAsset > templateScene = pipelineDepends->getObjectReadOnly< SceneAsset >(scenePermutationAsset->m_scene);
	if (!templateScene)
	{
		log::error << L"Scene permutation pipeline failed; Unable to read scene template" << Endl;
		return false;
	}

	pipelineDepends->addDependency(scenePermutationAsset->m_scene, editor::PdfUse);

	if (scenePermutationAsset->m_overridePostProcessSettings)
		pipelineDepends->addDependency(scenePermutationAsset->m_overridePostProcessSettings, editor::PdfBuild | editor::PdfResource);

	if (scenePermutationAsset->m_overrideWorldRenderSettings)
		pipelineDepends->addDependency(scenePermutationAsset->m_overrideWorldRenderSettings->reflectionMap, editor::PdfBuild | editor::PdfResource);

	const SmallMap< std::wstring, resource::Id< render::ITexture > >& params = scenePermutationAsset->m_overridePostProcessParams;
	for (SmallMap< std::wstring, resource::Id< render::ITexture > >::const_iterator i = params.begin(); i != params.end(); ++i)
		pipelineDepends->addDependency(i->second, editor::PdfBuild | editor::PdfResource);

	return true;
}

bool ScenePermutationPipeline::buildOutput(
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
	Ref< const SceneAsset > scenePermutation = checked_type_cast< const SceneAsset*, true >(buildOutput(pipelineBuilder, sourceAsset));
	if (!scenePermutation)
	{
		log::error << L"Scene permutation pipeline failed; unable to generate scene permutation" << Endl;
		return false;
	}

	return pipelineBuilder->buildOutput(
		scenePermutation,
		outputPath,
		outputGuid
	);
}

Ref< ISerializable > ScenePermutationPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	const ScenePermutationAsset* scenePermutationAsset = checked_type_cast< const ScenePermutationAsset*, false >(sourceAsset);
	
	Ref< const SceneAsset > templateScene = pipelineBuilder->getObjectReadOnly< SceneAsset >(scenePermutationAsset->m_scene);
	T_ASSERT (templateScene);

	const std::list< std::wstring >& includeLayers = scenePermutationAsset->m_includeLayers;

	Ref< SceneAsset > scenePermutation = DeepClone(templateScene).create< SceneAsset >();
	T_ASSERT (scenePermutation);

	if (scenePermutationAsset->m_overrideWorldRenderSettings)
		scenePermutation->setWorldRenderSettings(scenePermutationAsset->m_overrideWorldRenderSettings);

	if (scenePermutationAsset->m_overridePostProcessSettings)
		scenePermutation->setPostProcessSettings(scenePermutationAsset->m_overridePostProcessSettings);

	SmallMap< std::wstring, resource::Id< render::ITexture > > params = templateScene->getPostProcessParams();

	const SmallMap< std::wstring, resource::Id< render::ITexture > >& overrideParams = scenePermutationAsset->m_overridePostProcessParams;
	for (SmallMap< std::wstring, resource::Id< render::ITexture > >::const_iterator i = overrideParams.begin(); i != overrideParams.end(); ++i)
		params[i->first] = i->second;
	
	scenePermutation->setPostProcessParams(params);

	const RefArray< world::LayerEntityData >& layers = scenePermutation->getLayers();
	for (RefArray< world::LayerEntityData >::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (std::find(includeLayers.begin(), includeLayers.end(), (*i)->getName()) != includeLayers.end())
		{
			if (!(*i)->isInclude())
				(*i)->setInclude(true);
			else
				log::warning << L"Layer \"" << (*i)->getName() << L"\" already marked for \"include\" in scene; inconsistent permutation?" << Endl;
		}
	}

	return scenePermutation;
}

	}
}
