/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/OutputStream.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
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

void PipelineDependency::dump(OutputStream& os) const
{
	uint32_t i = 0;

	os << L"pipelineType: " << (pipelineType ? pipelineType->getName() : L"(null)") << Endl;
	os << L"sourceInstanceGuid: " << sourceInstanceGuid.format() << Endl;
	os << L"sourceAsset: " << type_name(sourceAsset) << Endl;

	for (external_files_t::const_iterator it = files.begin(); it != files.end(); ++it, ++i)
	{
		os << L"files[" << i << L"]" << Endl;
		os << L"\t.filePath: " << it->filePath.getPathName() << Endl;
		os << L"\t.lastWriteTime: " << it->lastWriteTime.getSecondsSinceEpoch() << Endl;
	}

	os << L"outputPath: " << outputPath << Endl;
	os << L"outputGuid: " << outputGuid.format() << Endl;
	os << L"pipelineHash: " << pipelineHash << Endl;
	os << L"sourceAssetHash: " << sourceAssetHash << Endl;
	os << L"sourceDataHash: " << sourceDataHash << Endl;
	os << L"filesHash: " << filesHash << Endl;

	os << L"flags:";
	if (flags & PdfBuild)
		os << L" PdfBuild";
	if (flags & PdfUse)
		os << L" PdfUse";
	if (flags & PdfResource)
		os << L" PdfResource";
	if (flags & PdfFailed)
		os << L" PdfFailed";
	os << Endl;

	for (i = 0; i < uint32_t(children.size()); ++i)
		os << L"children[" << i << L"] = " << children[i] << Endl;
}

void PipelineDependency::serialize(ISerializer& s)
{
	s >> MemberType(L"pipelineType", pipelineType);
	s >> Member< Guid >(L"sourceInstanceGuid", sourceInstanceGuid);
	s >> MemberRef< const ISerializable >(L"sourceAsset", sourceAsset);
	s >> MemberStlList< ExternalFile, MemberComposite< ExternalFile > >(L"files", files);
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
