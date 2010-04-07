#ifndef traktor_online_AchievementsPipeline_H
#define traktor_online_AchievementsPipeline_H

#include "Editor/DefaultPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

/*! \brief Achievements pipeline.
 * \ingroup Online
 */
class T_DLLCLASS AchievementsPipeline : public editor::DefaultPipeline
{
	T_RTTI_CLASS;

public:
	virtual bool create(const editor::IPipelineSettings* settings);

	virtual TypeInfoSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		Ref< const Object >& outBuildParams
	) const;

	virtual bool buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset,
		uint32_t sourceAssetHash,
		const Object* buildParams,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t reason
	) const;

private:
	std::wstring m_assetPath;
	std::wstring m_descriptorPath;
};

	}
}

#endif	// traktor_online_AchievementsPipeline_H
