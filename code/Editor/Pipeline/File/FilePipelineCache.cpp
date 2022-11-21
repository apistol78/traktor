/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/OutputStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/Pipeline/File/FilePipelineCache.h"
#include "Editor/Pipeline/File/FilePipelinePutStream.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.FilePipelineCache", FilePipelineCache, IPipelineCache)

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
		return nullptr;

	// Format guid as string and use two first, textual, bytes as subfolders.
	const std::wstring gs = guid.format();
	const std::wstring fa = gs.substr(1, 2);
	const std::wstring fb = gs.substr(3, 2);

	// Concate output path name.
	StringOutputStream ss;
	ss << m_path << L"/" << fa << L"/" << fb << L"/" << gs << L"_" << hash.pipelineHash << L"_" << hash.sourceAssetHash << L"_" << hash.sourceDataHash << L"_" << hash.filesHash << L".cache";

	// Open cached file.
	Ref< IStream > fileStream = FileSystem::getInstance().open(ss.str(), File::FmRead | File::FmMapped);
	if (!fileStream)
	{
		m_misses++;
		return nullptr;
	}

	m_hits++;
	return fileStream;
}

Ref< IStream > FilePipelineCache::put(const Guid& guid, const PipelineDependencyHash& hash)
{
	if (!m_accessWrite)
		return nullptr;

	// Format guid as string and use two first, textual, bytes as subfolders.
	const std::wstring gs = guid.format();
	const std::wstring fa = gs.substr(1, 2);
	const std::wstring fb = gs.substr(3, 2);

	// Concate output path name.
	StringOutputStream ss;
	ss << m_path << L"/" << fa << L"/" << fb << L"/" << gs << L"_" << hash.pipelineHash << L"_" << hash.sourceAssetHash << L"_" << hash.sourceDataHash << L"_" << hash.filesHash << L".cache";
	Path p(ss.str());

	// Ensure output path exists.
	if (!FileSystem::getInstance().makeAllDirectories(p.getPathOnly()))
	{
		log::error << L"File pipeline cache failed; unable to create cache directory for " << p.getPathName() << L"." << Endl;
		return nullptr;
	}

	// Create cached file, will be renamed when stream has been closed.
	Ref< IStream > fileStream = FileSystem::getInstance().open(p.getPathName() + L"~", File::FmWrite);
	if (!fileStream)
	{
		log::error << L"File pipeline cache failed; unable to create cache entry " << p.getPathName() << L"." << Endl;
		return nullptr;
	}

	return new FilePipelinePutStream(
		new BufferedStream(fileStream),
		ss.str()
	);
}

Ref< IStream > FilePipelineCache::get(const Key& key)
{
	StringOutputStream ss;
	ss << m_path << L"/" << key.format() << L".cache";
	const Path p(ss.str());

	// Ensure output path exists.
	if (!FileSystem::getInstance().makeAllDirectories(p.getPathOnly()))
	{
		log::error << L"File pipeline cache failed; unable to create cache directory for " << p.getPathName() << L"." << Endl;
		return nullptr;
	}

	// Open cached file.
	return FileSystem::getInstance().open(p.getPathName(), File::FmRead);
}

Ref< IStream > FilePipelineCache::put(const Key& key)
{
	StringOutputStream ss;
	ss << m_path << L"/" << key.format() << L".cache";
	const Path p(ss.str());

	// Ensure output path exists.
	if (!FileSystem::getInstance().makeAllDirectories(p.getPathOnly()))
	{
		log::error << L"File pipeline cache failed; unable to create cache directory for " << p.getPathName() << L"." << Endl;
		return nullptr;
	}

	// Create cached file, will be renamed when stream has been closed.
	Ref< IStream > fileStream = FileSystem::getInstance().open(p.getPathName() + L"~", File::FmWrite);
	if (!fileStream)
	{
		log::error << L"File pipeline cache failed; unable to create cache entry " << p.getPathName() << L"." << Endl;
		return nullptr;
	}

	return fileStream;
}

void FilePipelineCache::getInformation(OutputStream& os)
{
	os << L"File cache (";
	if (m_accessRead && !m_accessWrite)
		os << L"read";
	else if (!m_accessRead && m_accessWrite)
		os << L"write";
	else if (m_accessRead && m_accessWrite)
		os << L"read+write";
	else
		os << L"disabled";
	if (m_accessRead)
		os << L", " << m_hits << L" hits, " << m_misses << L" misses";
	os << L")";
}

bool FilePipelineCache::commit(const Guid& guid, const PipelineDependencyHash& hash)
{
	return true;
}

}
