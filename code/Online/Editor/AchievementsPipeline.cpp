#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Online/Editor/AchievementDesc.h"
#include "Online/Editor/AchievementsAsset.h"
#include "Online/Editor/AchievementsPipeline.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.AchievementsPipeline", 0, AchievementsPipeline, editor::DefaultPipeline)

bool AchievementsPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	m_descriptorPath = settings->getProperty< PropertyString >(L"AchievementsPipeline.DescriptorPath", L"");
	return true;
}

TypeInfoSet AchievementsPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< AchievementsAsset >());
	return typeSet;
}

bool AchievementsPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const AchievementsAsset > asset = checked_type_cast< const AchievementsAsset* >(sourceAsset);
	const RefArray< const AchievementDesc >& achievements = asset->get();
	for (RefArray< const AchievementDesc >::const_iterator i = achievements.begin(); i != achievements.end(); ++i)
	{
		Path fileName1 = FileSystem::getInstance().getAbsolutePath(m_assetPath, (*i)->getUnachievedImage());
		pipelineDepends->addDependency(fileName1);

		Path fileName2 = FileSystem::getInstance().getAbsolutePath(m_assetPath, (*i)->getAchievedImage());
		pipelineDepends->addDependency(fileName2);
	}
	return true;
}

bool AchievementsPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	if (!m_descriptorPath.empty())
	{
		// Create Steam descriptor.

		// Create PSN descriptor.

		// Create Xbox Live descriptor.
	}

	// Create runtime descriptor.

	return true;
}

	}
}
