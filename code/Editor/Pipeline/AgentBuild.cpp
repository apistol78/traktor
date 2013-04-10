#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Editor/Pipeline/AgentBuild.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.AgentBuild", 0, AgentBuild, ISerializable)

AgentBuild::AgentBuild()
:	m_sourceAssetHash(0)
,	m_reason(0)
{
}

AgentBuild::AgentBuild(
	const std::wstring& pipelineTypeName,
	const Guid& sourceInstanceGuid,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
)
:	m_pipelineTypeName(pipelineTypeName)
,	m_sourceInstanceGuid(sourceInstanceGuid)
,	m_sourceAsset(sourceAsset)
,	m_sourceAssetHash(sourceAssetHash)
,	m_outputPath(outputPath)
,	m_outputGuid(outputGuid)
,	m_reason(reason)
{
}

const std::wstring& AgentBuild::getPipelineTypeName() const
{
	return m_pipelineTypeName;
}

const Guid& AgentBuild::getSourceInstanceGuid() const
{
	return m_sourceInstanceGuid;
}

const ISerializable* AgentBuild::getSourceAsset() const
{
	return m_sourceAsset;
}

uint32_t AgentBuild::getSourceAssetHash() const
{
	return m_sourceAssetHash;
}

const std::wstring& AgentBuild::getOutputPath() const
{
	return m_outputPath;
}

const Guid& AgentBuild::getOutputGuid() const
{
	return m_outputGuid;
}

uint32_t AgentBuild::getReason() const
{
	return m_reason;
}

bool AgentBuild::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"pipelineTypeName", m_pipelineTypeName);
	s >> Member< Guid >(L"sourceInstanceGuid", m_sourceInstanceGuid);
	s >> MemberRef< const ISerializable >(L"sourceAsset", m_sourceAsset);
	s >> Member< uint32_t >(L"sourceAssetHash", m_sourceAssetHash);
	s >> Member< std::wstring >(L"outputPath", m_outputPath);
	s >> Member< Guid >(L"outputGuid", m_outputGuid);
	s >> Member< uint32_t >(L"reason", m_reason);
	return true;
}

	}
}
