#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
//#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberSmallSet.h"
#include "Core/Serialization/MemberType.h"
#include "Editor/PipelineDependency.h"

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
{
}

void PipelineDependency::dump() const
{
	log::info << L"pipelineType: " << (pipelineType ? pipelineType->getName() : L"(null)") << Endl;
	log::info << L"sourceInstanceGuid: " << sourceInstanceGuid.format() << Endl;
	log::info << L"sourceAsset: " << type_name(sourceAsset) << Endl;

	for (uint32_t i = 0; i < uint32_t(files.size()); ++i)
	{
		log::info << L"files[" << i << L"]" << Endl;
		log::info << L"\t.filePath: " << files[i].filePath.getPathName() << Endl;
		log::info << L"\t.lastWriteTime: " << files[i].lastWriteTime.getSecondsSinceEpoch() << Endl;
	}

	log::info << L"outputPath: " << outputPath << Endl;
	log::info << L"outputGuid: " << outputGuid.format() << Endl;
	log::info << L"pipelineHash: " << pipelineHash << Endl;
	log::info << L"sourceAssetHash: " << sourceAssetHash << Endl;
	log::info << L"sourceDataHash: " << sourceDataHash << Endl;
	log::info << L"filesHash: " << filesHash << Endl;

	log::info << L"flags:";
	if (flags & PdfBuild)
		log::info << L" PdfBuild";
	if (flags & PdfUse)
		log::info << L" PdfUse";
	if (flags & PdfResource)
		log::info << L" PdfResource";
	if (flags & PdfFailed)
		log::info << L" PdfFailed";
	log::info << Endl;

	for (uint32_t i = 0; i < uint32_t(children.size()); ++i)
		log::info << L"children[" << i << L"] = " << children[i] << Endl;
}

void PipelineDependency::serialize(ISerializer& s)
{
	s >> MemberType(L"pipelineType", pipelineType);
	s >> Member< Guid >(L"sourceInstanceGuid", sourceInstanceGuid);
	s >> MemberRef< const ISerializable >(L"sourceAsset", sourceAsset);
	s >> MemberAlignedVector< ExternalFile, MemberComposite< ExternalFile > >(L"files", files);
	s >> Member< std::wstring >(L"outputPath", outputPath);
	s >> Member< Guid >(L"outputGuid", outputGuid);
	s >> Member< uint32_t >(L"pipelineHash", pipelineHash);
	s >> Member< uint32_t >(L"sourceAssetHash", sourceAssetHash);
	s >> Member< uint32_t >(L"sourceDataHash", sourceDataHash);
	s >> Member< uint32_t >(L"filesHash", filesHash);
	s >> Member< uint32_t >(L"flags", flags);
	s >> MemberSmallSet< uint32_t >(L"children", children);
}

void PipelineDependency::ExternalFile::serialize(ISerializer& s)
{
	s >> Member< Path >(L"filePath", filePath);
	s >> MemberComposite< DateTime >(L"lastWriteTime", lastWriteTime);
}

	}
}
