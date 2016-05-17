#ifndef traktor_editor_AgentBuild_H
#define traktor_editor_AgentBuild_H

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

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

/*! \brief
 * \ingroup Editor
 */
class T_DLLCLASS AgentBuild : public ISerializable
{
	T_RTTI_CLASS;

public:
	AgentBuild();

	AgentBuild(
		const std::wstring& pipelineTypeName,
		const Guid& sourceInstanceGuid,
		const ISerializable* sourceAsset,
		uint32_t sourceAssetHash,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t reason
	);

	const std::wstring& getPipelineTypeName() const;

	const Guid& getSourceInstanceGuid() const;

	const ISerializable* getSourceAsset() const;

	uint32_t getSourceAssetHash() const;

	const std::wstring& getOutputPath() const;

	const Guid& getOutputGuid() const;

	uint32_t getReason() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_pipelineTypeName;
	Guid m_sourceInstanceGuid;
	Ref< const ISerializable > m_sourceAsset;
	uint32_t m_sourceAssetHash;
	std::wstring m_outputPath;
	Guid m_outputGuid;
	uint32_t m_reason;
};

	}
}

#endif	// traktor_editor_AgentBuild_H
