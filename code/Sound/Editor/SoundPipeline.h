#ifndef traktor_sound_SoundPipeline_H
#define traktor_sound_SoundPipeline_H

#include "Editor/IPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS SoundPipeline : public editor::IPipeline
{
	T_RTTI_CLASS(SoundPipeline)

public:
	virtual bool create(const editor::IPipelineSettings* settings);

	virtual void destroy();

	virtual uint32_t getVersion() const;

	virtual TypeSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const Serializable* sourceAsset,
		Ref< const Object >& outBuildParams
	) const;

	virtual bool buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const Serializable* sourceAsset,
		uint32_t sourceAssetHash,
		const Object* buildParams,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t reason
	) const;

private:
	std::wstring m_assetPath;
};

	}
}

#endif	// traktor_sound_SoundPipeline_H
