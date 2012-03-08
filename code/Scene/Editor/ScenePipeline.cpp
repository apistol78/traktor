#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/SceneResource.h"
#include "Scene/Editor/ScenePipeline.h"
#include "Scene/Editor/SceneAsset.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/EntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.ScenePipeline", 6, ScenePipeline, editor::IPipeline)

ScenePipeline::ScenePipeline()
:	m_suppressDepthPass(false)
,	m_suppressShadows(false)
,	m_suppressPostProcess(false)
,	m_suppressPreLit(false)
,	m_shadowMapSizeDenom(1)
{
}

bool ScenePipeline::create(const editor::IPipelineSettings* settings)
{
	m_suppressDepthPass = settings->getProperty< PropertyBoolean >(L"ScenePipeline.SuppressDepthPass");
	m_suppressShadows = settings->getProperty< PropertyBoolean >(L"ScenePipeline.SuppressShadows");
	m_suppressPostProcess = settings->getProperty< PropertyBoolean >(L"ScenePipeline.SuppressPostProcess");
	m_suppressPreLit = settings->getProperty< PropertyBoolean >(L"ScenePipeline.SuppressPreLit");
	m_shadowMapSizeDenom = settings->getProperty< PropertyBoolean >(L"ScenePipeline.ShadowMapSizeDenom");
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
	Ref< const Object >& outBuildParams
) const
{
	Ref< const SceneAsset > sceneAsset = checked_type_cast< const SceneAsset* >(sourceAsset);
	pipelineDepends->addDependency(sceneAsset->getPostProcessSettings().getGuid(), editor::PdfBuild);
	pipelineDepends->addDependency(sceneAsset->getEntityData());
	pipelineDepends->addDependency(sceneAsset->getControllerData());
	return true;
}

bool ScenePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< SceneAsset > sceneAsset = DeepClone(sourceAsset).create< SceneAsset >();
	Ref< SceneResource > sceneResource = new SceneResource();

	Ref< world::EntityData > entityData = checked_type_cast< world::EntityData*, true >(pipelineBuilder->buildOutput(sceneAsset->getEntityData()));
	if (!entityData)
		return false;

	Ref< ISceneControllerData > controllerData = checked_type_cast< ISceneControllerData*, true >(pipelineBuilder->buildOutput(sceneAsset->getControllerData()));

	sceneResource->setWorldRenderSettings(sceneAsset->getWorldRenderSettings());
	sceneResource->setPostProcessSettings(sceneAsset->getPostProcessSettings());
	sceneResource->setEntityData(entityData);
	sceneResource->setControllerData(controllerData);

	if (m_suppressDepthPass && sceneResource->getWorldRenderSettings()->depthPassEnabled)
	{
		sceneResource->getWorldRenderSettings()->depthPassEnabled = false;
		log::info << L"Depth pass suppressed" << Endl;
	}
	if (m_suppressShadows && sceneResource->getWorldRenderSettings()->shadowsEnabled)
	{
		sceneResource->getWorldRenderSettings()->shadowsEnabled = false;
		log::info << L"Shadows suppressed" << Endl;
	}
	if (m_suppressPostProcess && !sceneResource->getPostProcessSettings().getGuid().isNull())
	{
		sceneResource->setPostProcessSettings(Guid());
		log::info << L"Post processing suppressed" << Endl;
	}
	if (m_suppressPreLit && sceneResource->getWorldRenderSettings()->renderType == world::WorldRenderSettings::RtPreLit)
	{
		sceneResource->getWorldRenderSettings()->renderType = world::WorldRenderSettings::RtForward;
		log::info << L"PreLit render type suppressed" << Endl;
	}

	if (m_shadowMapSizeDenom > 1)
	{
		sceneResource->getWorldRenderSettings()->shadowMapResolution =
			sceneAsset->getWorldRenderSettings()->shadowMapResolution / m_shadowMapSizeDenom;
		log::info << L"Reduced shadow map size " << sceneResource->getWorldRenderSettings()->shadowMapResolution << Endl;
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
	T_FATAL_ERROR;
	return 0;
}

	}
}
