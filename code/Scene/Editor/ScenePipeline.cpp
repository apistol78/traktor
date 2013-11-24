#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/SceneResource.h"
#include "Scene/Editor/ScenePipeline.h"
#include "Scene/Editor/SceneAsset.h"
#include "World/WorldRenderSettings.h"
#include "World/Editor/LayerEntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.ScenePipeline", 9, ScenePipeline, editor::IPipeline)

ScenePipeline::ScenePipeline()
:	m_targetEditor(false)
,	m_suppressLinearLighting(false)
,	m_suppressDepthPass(false)
,	m_suppressPostProcess(false)
,	m_shadowMapSizeDenom(1)
,	m_shadowMapMaxSlices(0)
{
}

bool ScenePipeline::create(const editor::IPipelineSettings* settings)
{
	m_targetEditor = settings->getProperty< PropertyBoolean >(L"Pipeline.TargetEditor");
	m_suppressLinearLighting = settings->getProperty< PropertyBoolean >(L"ScenePipeline.SuppressLinearLighting");
	m_suppressDepthPass = settings->getProperty< PropertyBoolean >(L"ScenePipeline.SuppressDepthPass");
	m_suppressPostProcess = settings->getProperty< PropertyBoolean >(L"ScenePipeline.SuppressPostProcess");
	m_shadowMapSizeDenom = settings->getProperty< PropertyInteger >(L"ScenePipeline.ShadowMapSizeDenom", 1);
	m_shadowMapMaxSlices = settings->getProperty< PropertyInteger >(L"ScenePipeline.ShadowMapMaxSlices", 0);
	return true;
}

void ScenePipeline::destroy()
{
}

TypeInfoSet ScenePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SceneAsset >());
	return typeSet;
}

bool ScenePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< const SceneAsset > sceneAsset = checked_type_cast< const SceneAsset* >(sourceAsset);
	pipelineDepends->addDependency(sceneAsset->getPostProcessSettings(), editor::PdfBuild | editor::PdfResource);

	const RefArray< world::LayerEntityData >& layers = sceneAsset->getLayers();
	for (RefArray< world::LayerEntityData >::const_iterator i = layers.begin(); i != layers.end(); ++i)
		pipelineDepends->addDependency(*i);

	pipelineDepends->addDependency(sceneAsset->getControllerData());

	if (sceneAsset->getWorldRenderSettings())
		pipelineDepends->addDependency(sceneAsset->getWorldRenderSettings()->reflectionMap, editor::PdfBuild | editor::PdfResource);

	return true;
}

bool ScenePipeline::buildOutput(
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
	const SceneAsset* sceneAsset = checked_type_cast< const SceneAsset*, false >(sourceAsset);

	Ref< world::GroupEntityData > groupEntityData = new world::GroupEntityData();

	// Build each layer of entity data; merge into a single output group.
	const RefArray< world::LayerEntityData >& layers = sceneAsset->getLayers();
	for (RefArray< world::LayerEntityData >::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (!(*i))
			continue;

		if ((*i)->isInclude() || m_targetEditor)
		{
			log::info << L"Building layer \"" << (*i)->getName() << L"\"..." << Endl;
			const RefArray< world::EntityData >& entityData = (*i)->getEntityData();
			for (RefArray< world::EntityData >::const_iterator j = entityData.begin(); j != entityData.end(); ++j)
			{
				Ref< world::EntityData > entityData = checked_type_cast< world::EntityData*, true >(pipelineBuilder->buildOutput(*j));
				if (entityData)
					groupEntityData->addEntityData(entityData);
			}
		}
		else
			log::info << L"Layer \"" << (*i)->getName() << L"\" skipped" << Endl;
	}

	// Build controller data.
	Ref< ISceneControllerData > controllerData = checked_type_cast< ISceneControllerData*, true >(pipelineBuilder->buildOutput(sceneAsset->getControllerData()));

	Ref< SceneResource > sceneResource = new SceneResource();
	sceneResource->setWorldRenderSettings(sceneAsset->getWorldRenderSettings());
	sceneResource->setPostProcessSettings(sceneAsset->getPostProcessSettings());
	sceneResource->setEntityData(groupEntityData);
	sceneResource->setControllerData(controllerData);

	if (m_suppressLinearLighting && sceneResource->getWorldRenderSettings()->linearLighting)
	{
		sceneResource->getWorldRenderSettings()->linearLighting = false;
		log::info << L"Linear lighting suppressed" << Endl;
	}
	if (m_suppressDepthPass && sceneResource->getWorldRenderSettings()->depthPassEnabled)
	{
		sceneResource->getWorldRenderSettings()->depthPassEnabled = false;
		log::info << L"Depth pass suppressed" << Endl;
	}
	if (m_suppressPostProcess && !sceneResource->getPostProcessSettings().isNull())
	{
		sceneResource->setPostProcessSettings(resource::Id< world::PostProcessSettings >());
		log::info << L"Post processing suppressed" << Endl;
	}

	for (uint32_t i = 0; i < world::QuLast; ++i)
	{
		world::WorldRenderSettings::ShadowSettings& shadowSetting = sceneResource->getWorldRenderSettings()->shadowSettings[i];
		shadowSetting.resolution /= m_shadowMapSizeDenom;
		if (m_shadowMapMaxSlices > 0)
			shadowSetting.cascadingSlices = std::min(shadowSetting.cascadingSlices, m_shadowMapMaxSlices);
	}

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(sceneResource);

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > ScenePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	return DeepClone(sourceAsset).create< SceneAsset >();
}

	}
}
