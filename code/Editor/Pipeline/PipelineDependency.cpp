#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberType.h"
#include "Editor/Pipeline/PipelineDependency.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PipelineDependency", 0, PipelineDependency, ISerializable)

PipelineDependency::PipelineDependency()
:	pipelineType(0)
,	pipelineHash(0)
,	sourceAssetHash(0)
,	sourceDataHash(0)
,	filesHash(0)
,	flags(0)
,	reason(0)
{
}

void PipelineDependency::serialize(ISerializer& s)
{
	s >> MemberType(L"pipelineType", pipelineType);
	s >> Member< Guid >(L"sourceInstanceGuid", sourceInstanceGuid);
	s >> MemberComposite< DateTime >(L"sourceInstanceLastModifyDate", sourceInstanceLastModifyDate);
	s >> MemberRef< const ISerializable >(L"sourceAsset", sourceAsset);
	s >> MemberStlVector< ExternalFile, MemberComposite< ExternalFile > >(L"files", files);
	s >> Member< std::wstring >(L"outputPath", outputPath);
	s >> Member< Guid >(L"outputGuid", outputGuid);
	s >> Member< uint32_t >(L"pipelineHash", pipelineHash);
	s >> Member< uint32_t >(L"sourceAssetHash", sourceAssetHash);
	s >> Member< uint32_t >(L"sourceDataHash", sourceDataHash);
	s >> Member< uint32_t >(L"filesHash", filesHash);
	s >> Member< uint32_t >(L"flags", flags);
	s >> Member< uint32_t >(L"reason", reason);
	s >> MemberRefArray< PipelineDependency >(L"children", children);
}

void PipelineDependency::ExternalFile::serialize(ISerializer& s)
{
	s >> Member< Path >(L"filePath", filePath);
	s >> MemberComposite< DateTime >(L"lastWriteTime", lastWriteTime);
}

	}
}
