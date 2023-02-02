/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Core/Thread/Acquire.h"
#include "Editor/Pipeline/PipelineDbFlat.h"

namespace traktor::editor
{
	namespace
	{

const uint32_t c_version = 3;
const uint32_t c_flushAfterChanges = 100;	//!< Flush pipeline after N changes.

class MemberPipelineDependencyHash : public MemberComplex
{
public:
	MemberPipelineDependencyHash(const wchar_t* const name, PipelineDependencyHash& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	void serialize(ISerializer& s) const
	{
		s >> Member< uint32_t >(L"pipelineHash", m_ref.pipelineHash);
		s >> Member< uint32_t >(L"sourceAssetHash", m_ref.sourceAssetHash);
		s >> Member< uint32_t >(L"sourceDataHash", m_ref.sourceDataHash);
		s >> Member< uint32_t >(L"filesHash", m_ref.filesHash);
	}

private:
	PipelineDependencyHash& m_ref;
};

class MemberPipelineFileHash : public MemberComplex
{
public:
	MemberPipelineFileHash(const wchar_t* const name, PipelineFileHash& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	void serialize(ISerializer& s) const
	{
		s >> Member< uint64_t >(L"size", m_ref.size);
		s >> MemberComposite< DateTime >(L"lastWriteTime", m_ref.lastWriteTime);
		s >> Member< uint32_t >(L"hash", m_ref.hash);
	}

private:
	PipelineFileHash& m_ref;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDbFlat", PipelineDbFlat, IPipelineDb)

bool PipelineDbFlat::open(const std::wstring& connectionString)
{
	std::vector< std::wstring > pairs;
	std::map< std::wstring, std::wstring > cs;

	if (Split< std::wstring >::any(connectionString, L";", pairs) == 0)
	{
		log::error << L"Unable to open pipeline db; incorrect connection string." << Endl;
		return false;
	}

	for (const auto& pair : pairs)
	{
		size_t p = pair.find(L'=');
		if (p == 0 || p == pair.npos)
		{
			log::error << L"Unable to open pipeline db; incorrect connection string." << Endl;
			return false;
		}

		cs[trim(pair.substr(0, p))] = pair.substr(p + 1);
	}

	m_file = cs[L"fileName"];

	// If flat database file doesn't exist we assume this is the first run; ie. don't fail.
	if (!FileSystem::getInstance().exist(m_file))
	{
		// But ensure full path is created first.
		return FileSystem::getInstance().makeAllDirectories(Path(m_file).getPathOnly());
	}

	Ref< IStream > f = FileSystem::getInstance().open(m_file, File::FmRead);
	if (!f)
	{
		log::error << L"Unable to open pipeline db; failed to open file." << Endl;
		return false;
	}

	BinarySerializer s(f);

	uint32_t version = 0;
	s >> Member< uint32_t >(L"version", version);

	if (version == c_version)
	{
		s >> MemberSmallMap<
			Guid,
			PipelineDependencyHash,
			Member< Guid >,
			MemberPipelineDependencyHash
		>(L"dependencies", m_dependencies);

		s >> MemberSmallMap<
			std::wstring,
			PipelineFileHash,
			Member< std::wstring >,
			MemberPipelineFileHash
		>(L"files", m_files);
	}
	else
		log::warning << L"Pipeline database version mismatch; database purged and rebuild is required." << Endl;

	f->close();
	f = nullptr;

	return true;
}

void PipelineDbFlat::close()
{
	endTransaction();
}

void PipelineDbFlat::beginTransaction()
{
	T_FATAL_ASSERT(m_changes == 0);
}

void PipelineDbFlat::endTransaction()
{
	if (m_changes > 0)
	{
		Ref< IStream > f = FileSystem::getInstance().open(m_file, File::FmWrite);
		if (f)
		{
			BufferedStream bs(f);
			BinarySerializer s(&bs);

			uint32_t version = c_version;
			s >> Member< uint32_t >(L"version", version);

			s >> MemberSmallMap<
				Guid,
				PipelineDependencyHash,
				Member< Guid >,
				MemberPipelineDependencyHash
			>(L"dependencies", m_dependencies);

			s >> MemberSmallMap<
				std::wstring,
				PipelineFileHash,
				Member< std::wstring >,
				MemberPipelineFileHash
			>(L"files", m_files);

			bs.close();
			f = nullptr;

			m_changes = 0;
		}
		else
			log::error << L"Unable to flush pipeline db; failed to write latest changes." << Endl;
	}
}

void PipelineDbFlat::setDependency(const Guid& guid, const PipelineDependencyHash& hash)
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireWriter)(m_lock);
	m_dependencies[guid] = hash;
	if (++m_changes >= c_flushAfterChanges)
		endTransaction();
}

bool PipelineDbFlat::getDependency(const Guid& guid, PipelineDependencyHash& outHash) const
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lock);
	auto it = m_dependencies.find(guid);
	if (it == m_dependencies.end())
		return false;
	outHash = it->second;
	return true;
}

void PipelineDbFlat::setFile(const Path& path, const PipelineFileHash& file)
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireWriter)(m_lock);
	m_files[path.getPathName()] = file;
	if (++m_changes >= c_flushAfterChanges)
		endTransaction();
}

bool PipelineDbFlat::getFile(const Path& path, PipelineFileHash& outFile) const
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lock);

	auto it = m_files.find(path.getPathName());
	if (it == m_files.end())
		return false;

	outFile = it->second;
	return true;
}

uint32_t PipelineDbFlat::getDependencyCount() const
{
	return (uint32_t)m_dependencies.size();
}

bool PipelineDbFlat::getDependencyByIndex(uint32_t index, Guid& outGuid, PipelineDependencyHash& outHash) const
{
	auto it = m_dependencies.begin();
	std::advance(it, index);
	outGuid = it->first;
	outHash = it->second;
	return true;
}

uint32_t PipelineDbFlat::getFileCount() const
{
	return (uint32_t)m_files.size();
}

bool PipelineDbFlat::getFileByIndex(uint32_t index, Path& outPath, PipelineFileHash& outFile) const
{
	auto it = m_files.begin();
	std::advance(it, index);
	outPath = it->first;
	outFile = it->second;
	return true;
}

}
