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
#include "Scene/Editor/IScenePipelineOperator.h"
#include "Scene/Editor/ScenePipeline.h"
#include "Scene/Editor/SceneAsset.h"
#include "World/WorldRenderSettings.h"
#include "World/Editor/LayerEntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.ScenePipeline", 12, ScenePipeline, editor::IPipeline)

ScenePipeline::ScenePipeline()
:	m_targetEditor(false)
,	m_suppressShadows(false)
,	m_suppressLinearLighting(false)
,	m_suppressDepthPass(false)
,	m_suppressImageProcess(false)
,	m_shadowMapSizeDenom(1)
,	m_shadowMapMaxSlices(0)
{
}

bool ScenePipeline::create(const editor::IPipelineSettings* settings)
{
	m_targetEditor = settings->getProperty< bool >(L"Pipeline.TargetEditor");
	m_suppressShadows = settings->getProperty< bool >(L"ScenePipeline.SuppressShadows");
	m_suppressLinearLighting = settings->getProperty< bool >(L"ScenePipeline.SuppressLinearLighting");
	m_suppressDepthPass = settings->getProperty< bool >(L"ScenePipeline.SuppressDepthPass");
	m_suppressImageProcess = settings->getProperty< bool >(L"ScenePipeline.SuppressImageProcess");
	m_shadowMapSizeDenom = settings->getProperty< int32_t >(L"ScenePipeline.ShadowMapSizeDenom", 1);
	m_shadowMapMaxSlices = settings->getProperty< int32_t >(L"ScenePipeline.ShadowMapMaxSlices", 0);

	// Instantiate scene pipeline operators.
	TypeInfoSet operatorTypes;
	type_of< IScenePipelineOperator >().findAllOf(operatorTypes, false);
	for (auto operatorType : operatorTypes)
	{
		Ref< IScenePipelineOperator > spo = dynamic_type_cast< IScenePipelineOperator* >(operatorType->createInstance());
		if (!spo)
		{
			log::error << L"Failed to create scene pipeline; unable to instantiate operator \"" << operatorType->getName() << L"\"." << Endl;
			return false;
		}
		if (!spo->create(settings))
		{
			log::error << L"Failed to create scene pipeline; unable to create operator \"" << operatorType->getName() << L"\"." << Endl;
			return false;
		}
		m_operators.push_back(spo);
	}

	return true;
}

void ScenePipeline::destroy()
{
	for (auto op : m_operators)
		op->destroy();
	m_operators.clear();
}

TypeInfoSet ScenePipeline::getAssetTypes() const
{
	return makeTypeInfoSet< SceneAsset >();
}

bool ScenePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const SceneAsset* sceneAsset = mandatory_non_null_type_cast< const SceneAsset* >(sourceAsset);

	for (const auto& param : sceneAsset->getImageProcessParams())
		pipelineDepends->addDependency(param.second, editor::PdfBuild | editor::PdfResource);

	for (const auto& layer : sceneAsset->getLayers())
		pipelineDepends->addDependency(layer);

	pipelineDepends->addDependency(sceneAsset->getControllerData());

	const world::WorldRenderSettings* wrs = sceneAsset->getWorldRenderSettings();
	if (wrs)
	{
		if (!m_suppressShadows)
		{
			for (int32_t i = 0; i < sizeof_array(wrs->shadowSettings); ++i)
				pipelineDepends->addDependency(wrs->shadowSettings[i].maskProject, editor::PdfBuild | editor::PdfResource);
		}

		pipelineDepends->addDependency(wrs->irradianceGrid, editor::PdfBuild | editor::PdfResource);

		if (!m_suppressImageProcess)
		{
			for (int32_t i = 0; i < sizeof_array(wrs->imageProcess); ++i)
				pipelineDepends->addDependency(wrs->imageProcess[i], editor::PdfBuild | editor::PdfResource);
		}
	}

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
	// Create clone of asset which operators can modify.
	Ref< SceneAsset > sceneAsset = DeepClone(sourceAsset).create< SceneAsset >();
	T_FATAL_ASSERT (sceneAsset != nullptr);

	// Execute operations on scene.
	bool rebuild = (bool)((reason & editor::PbrForced) != 0);
	for (const auto op : sceneAsset->getOperationData())
	{
		const IScenePipelineOperator* spo = findOperator(type_of(op));
		if (!spo)
			return false;
		if (!spo->build(pipelineBuilder, op, sourceInstance, sceneAsset, rebuild))
			return false;
	}

	// Build each layer of entity data; merge into a single output group.
	Ref< world::GroupEntityData > groupEntityData = new world::GroupEntityData();
	for (const auto& layer : sceneAsset->getLayers())
	{
		if (!layer)
			continue;

		if (layer->isInclude() || m_targetEditor)
		{
			log::info << L"Building layer \"" << layer->getName() << L"\"..." << Endl;
			for (const auto& assetEntityData : layer->getEntityData())
			{
				Ref< world::EntityData > outputEntityData = checked_type_cast< world::EntityData*, true >(pipelineBuilder->buildOutput(sourceInstance, assetEntityData));
				if (outputEntityData)
					groupEntityData->addEntityData(outputEntityData);
			}
		}
		else
			log::info << L"Layer \"" << layer->getName() << L"\" skipped." << Endl;
	}

	// Build controller data.
	Ref< ISceneControllerData > controllerData = checked_type_cast< ISceneControllerData*, true >(pipelineBuilder->buildOutput(sourceInstance, sceneAsset->getControllerData()));

	// Create output scene resource.
	Ref< SceneResource > sceneResource = new SceneResource();
	sceneResource->setWorldRenderSettings(sceneAsset->getWorldRenderSettings());
	sceneResource->setImageProcessParams(sceneAsset->getImageProcessParams());
	sceneResource->setEntityData(groupEntityData);
	sceneResource->setControllerData(controllerData);

	if (m_suppressLinearLighting && sceneResource->getWorldRenderSettings()->linearLighting)
	{
		sceneResource->getWorldRenderSettings()->linearLighting = false;
		log::info << L"Linear lighting suppressed." << Endl;
	}

	for (uint32_t i = 0; i < (int32_t)world::Quality::Last; ++i)
	{
		if (m_suppressImageProcess)
			sceneResource->getWorldRenderSettings()->imageProcess[i] = resource::Id< render::ImageGraph >();
	}

	for (uint32_t i = 0; i < (int32_t)world::Quality::Last; ++i)
	{
		world::WorldRenderSettings::ShadowSettings& shadowSetting = sceneResource->getWorldRenderSettings()->shadowSettings[i];
		if (!m_suppressShadows)
		{
			shadowSetting.resolution /= m_shadowMapSizeDenom;
			if (m_shadowMapMaxSlices > 0)
				shadowSetting.cascadingSlices = std::min(shadowSetting.cascadingSlices, m_shadowMapMaxSlices);
		}
		else
			shadowSetting.maskProject = resource::Id< render::ImageGraph >();
	}

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance \"" << outputPath << L"\"." << Endl;
		return false;
	}

	outputInstance->setObject(sceneResource);

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance \"" << outputPath << L"\"." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > ScenePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	return DeepClone(sourceAsset).create< SceneAsset >();
}

const IScenePipelineOperator* ScenePipeline::findOperator(const TypeInfo& operationType) const
{
	for (const auto sop : m_operators)
	{
		for (const auto ts : sop->getOperatorTypes())
		{
			if (is_type_a(*ts, operationType))
				return sop;
		}
	}
	return nullptr;
}

	}
}
