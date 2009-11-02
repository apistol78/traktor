#include "Scene/Editor/ScenePipeline.h"
#include "Scene/SceneAsset.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/EntityInstance.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineSettings.h"
#include "Database/Instance.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.scene.ScenePipeline", ScenePipeline, editor::IPipeline)

ScenePipeline::ScenePipeline()
:	m_suppressDepthPass(false)
,	m_suppressVelocity(false)
,	m_suppressShadows(false)
,	m_suppressPostProcess(false)
{
}

bool ScenePipeline::create(const editor::IPipelineSettings* settings)
{
	m_suppressDepthPass = settings->getProperty< editor::PropertyBoolean >(L"ScenePipeline.SuppressDepthPass");
	m_suppressVelocity = settings->getProperty< editor::PropertyBoolean >(L"ScenePipeline.SuppressVelocityPass");
	m_suppressShadows = settings->getProperty< editor::PropertyBoolean >(L"ScenePipeline.SuppressShadows");
	m_suppressPostProcess = settings->getProperty< editor::PropertyBoolean >(L"ScenePipeline.SuppressPostProcess");
	return true;
}

void ScenePipeline::destroy()
{
}

uint32_t ScenePipeline::getVersion() const
{
	return 4;
}

TypeSet ScenePipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< SceneAsset >());
	return typeSet;
}

bool ScenePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const SceneAsset > sceneAsset = checked_type_cast< const SceneAsset* >(sourceAsset);
	pipelineDepends->addDependency(sceneAsset->getPostProcessSettings().getGuid(), true);
	pipelineDepends->addDependency(sceneAsset->getInstance());
	return true;
}

bool ScenePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const Serializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< SceneAsset > sceneAsset = DeepClone(sourceAsset).create< SceneAsset >();

	if (m_suppressDepthPass && sceneAsset->getWorldRenderSettings()->depthPassEnabled)
	{
		sceneAsset->getWorldRenderSettings()->depthPassEnabled = false;
		log::info << L"Depth pass suppressed" << Endl;
	}
	if (m_suppressVelocity && sceneAsset->getWorldRenderSettings()->velocityPassEnable)
	{
		sceneAsset->getWorldRenderSettings()->velocityPassEnable = false;
		log::info << L"Velocity pass suppressed" << Endl;
	}
	if (m_suppressShadows && sceneAsset->getWorldRenderSettings()->shadowsEnabled)
	{
		sceneAsset->getWorldRenderSettings()->shadowsEnabled = false;
		log::info << L"Shadows suppressed" << Endl;
	}
	if (m_suppressPostProcess && !sceneAsset->getPostProcessSettings().getGuid().isNull())
	{
		sceneAsset->setPostProcessSettings(Guid());
		log::info << L"Post processing suppressed" << Endl;
	}

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(sceneAsset);

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

	}
}
