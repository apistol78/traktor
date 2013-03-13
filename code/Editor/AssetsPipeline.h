#ifndef traktor_editor_AssetsPipeline_H
#define traktor_editor_AssetsPipeline_H

#include "Editor/IPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

/*! \brief Assets pipeline.
 * \ingroup Editor
 */
class T_DLLCLASS AssetsPipeline : public IPipeline
{
	T_RTTI_CLASS;

public:
	virtual bool create(const IPipelineSettings* settings);

	virtual void destroy();

	virtual TypeInfoSet getAssetTypes() const;

	virtual bool buildDependencies(
		IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const;

	virtual bool buildOutput(
		IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset,
		uint32_t sourceAssetHash,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		const Object* buildParams,
		uint32_t reason
	) const;

	virtual Ref< ISerializable > buildOutput(
		IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset
	) const;
};

	}
}

#endif	// traktor_editor_AssetsPipeline_H
