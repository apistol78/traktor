/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/Pipeline/FilePipelineCache.h"
#include "Editor/Pipeline/FilePipelinePutStream.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.FilePipelineCache", FilePipelineCache, IPipelineCache)

FilePipelineCache::FilePipelineCache()
:	m_accessRead(true)
,	m_accessWrite(true)
{
}

bool FilePipelineCache::create(const PropertyGroup* settings)
{
	m_accessRead = settings->getProperty< bool >(L"Pipeline.FileCache.Read", true);
	m_accessWrite = settings->getProperty< bool >(L"Pipeline.FileCache.Write", true);	
	m_path = settings->getProperty< std::wstring >(L"Pipeline.FileCache.Path");
	return true;
}

void FilePipelineCache::destroy()
{
}

Ref< IStream > FilePipelineCache::get(const Guid& guid, const PipelineDependencyHash& hash)
{
	if (!m_accessRead)
		return 0;

	// Format guid as string and use two first, textual, bytes as subfolders.
	std::wstring gs = guid.format();
	std::wstring fa = gs.substr(1, 2);
	std::wstring fb = gs.substr(3, 2);

	// Concate output path name.
	StringOutputStream ss;
	ss << m_path << L"/" << fa << L"/" << fb << L"/" << gs << L"_" << hash.pipelineHash << L"_" << hash.sourceAssetHash << L"_" << hash.sourceDataHash << L"_" << hash.filesHash << L".cache";

	// Open cached file.	
	Ref< IStream > fileStream = FileSystem::getInstance().open(ss.str(), File::FmRead);
	if (!fileStream)
		return 0;

	return new BufferedStream(fileStream);
}

Ref< IStream > FilePipelineCache::put(const Guid& guid, const PipelineDependencyHash& hash)
{
	if (!m_accessWrite)
		return 0;

	// Format guid as string and use two first, textual, bytes as subfolders.
	std::wstring gs = guid.format();
	std::wstring fa = gs.substr(1, 2);
	std::wstring fb = gs.substr(3, 2);

	// Concate output path name.
	StringOutputStream ss;
	ss << m_path << L"/" << fa << L"/" << fb << L"/" << gs << L"_" << hash.pipelineHash << L"_" << hash.sourceAssetHash << L"_" << hash.sourceDataHash << L"_" << hash.filesHash << L".cache";
	Path p(ss.str());

	// Ensure output path exists.
	if (!FileSystem::getInstance().makeAllDirectories(p.getPathOnly()))
	{
		log::error << L"File pipeline cache failed; unable to create cache directory for " << p.getPathName() << L"." << Endl;
		return 0;
	}

	// Create cached file, will be renamed when stream has been closed.
	Ref< IStream > fileStream = FileSystem::getInstance().open(p.getPathName() + L"~", File::FmWrite);
	if (!fileStream)
	{
		log::error << L"File pipeline cache failed; unable to create cache entry " << p.getPathName() << L"." << Endl;
		return 0;
	}

	return new FilePipelinePutStream(
		new BufferedStream(fileStream),
		ss.str()
	);
}

	}
}
