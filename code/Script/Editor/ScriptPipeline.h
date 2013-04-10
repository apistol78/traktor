#ifndef traktor_script_ScriptPipeline_H
#define traktor_script_ScriptPipeline_H

#include "Editor/DefaultPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class IScriptManager;

/*! \brief Script pipeline.
 * \ingroup Script
 */
class T_DLLCLASS ScriptPipeline : public editor::DefaultPipeline
{
	T_RTTI_CLASS;

public:
	virtual bool create(const editor::IPipelineSettings* settings);

	virtual TypeInfoSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const;

	virtual bool buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		uint32_t sourceAssetHash,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		const Object* buildParams,
		uint32_t reason
	) const;

private:
	Ref< IScriptManager > m_scriptManager;
	std::wstring m_scriptOutputPath;
};

	}
}

#endif	// traktor_script_ScriptPipeline_H
