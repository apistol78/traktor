#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Thread/Acquire.h"
#include "Editor/Pipeline/PipelineDbFlat.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

const uint32_t c_version = 2;
const uint32_t c_flushAfterChanges = 10;	//!< Flush pipeline after N changes.

class MemberPipelineDependencyHash : public MemberComplex
{
public:
	MemberPipelineDependencyHash(const wchar_t* const name, PipelineDependencyHash& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		s >> Member< uint32_t >(L"pipelineVersion", m_ref.pipelineVersion);
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

	virtual void serialize(ISerializer& s) const
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

PipelineDbFlat::PipelineDbFlat()
:	m_changes(0)
{
}

bool PipelineDbFlat::open(const std::wstring& connectionString)
{
	std::vector< std::wstring > pairs;
	std::map< std::wstring, std::wstring > cs;

	if (Split< std::wstring >::any(connectionString, L";", pairs) == 0)
	{
		log::error << L"Unable to open pipeline db; incorrect connection string." << Endl;
		return false;
	}

	for (std::vector< std::wstring >::const_iterator i = pairs.begin(); i != pairs.end(); ++i)
	{
		size_t p = i->find(L'=');
		if (p == 0 || p == i->npos)
		{
			log::error << L"Unable to open pipeline db; incorrect connection string." << Endl;
			return false;
		}

		cs[trim(i->substr(0, p))] = i->substr(p + 1);
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
		return false;

	BinarySerializer s(f);

	uint32_t version = 0;
	s >> Member< uint32_t >(L"version", version);

	if (version == c_version)
	{
		s >> MemberStlMap<
			Guid,
			PipelineDependencyHash,
			MemberStlPair<
				Guid,
				PipelineDependencyHash,
				Member< Guid >,
				MemberPipelineDependencyHash
			>
		>(L"dependencies", m_dependencies);

		s >> MemberStlMap<
			std::wstring,
			PipelineFileHash,
			MemberStlPair<
				std::wstring,
				PipelineFileHash,
				Member< std::wstring >,
				MemberPipelineFileHash
			>
		>(L"files", m_files);
	}

	f->close();
	f = 0;

	return true;
}

void PipelineDbFlat::close()
{
	endTransaction();
}

void PipelineDbFlat::beginTransaction()
{
}

void PipelineDbFlat::endTransaction()
{
	if (m_changes > 0)
	{
		Ref< IStream > f = FileSystem::getInstance().open(m_file, File::FmWrite);
		if (f)
		{
			BinarySerializer s(f);

			uint32_t version = c_version;
			s >> Member< uint32_t >(L"version", version);

			s >> MemberStlMap<
				Guid,
				PipelineDependencyHash,
				MemberStlPair<
					Guid,
					PipelineDependencyHash,
					Member< Guid >,
					MemberPipelineDependencyHash
				>
			>(L"dependencies", m_dependencies);

			s >> MemberStlMap<
				std::wstring,
				PipelineFileHash,
				MemberStlPair<
					std::wstring,
					PipelineFileHash,
					Member< std::wstring >,
					MemberPipelineFileHash
				>
			>(L"files", m_files);

			f->close();
			f = 0;

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

	std::map< Guid, PipelineDependencyHash >::const_iterator it = m_dependencies.find(guid);
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

bool PipelineDbFlat::getFile(const Path& path, PipelineFileHash& outFile)
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lock);

	std::map< std::wstring, PipelineFileHash >::const_iterator it = m_files.find(path.getPathName());
	if (it == m_files.end())
		return false;

	outFile = it->second;
	return true;
}

Ref< IPipelineReport > PipelineDbFlat::createReport(const std::wstring& name, const Guid& guid)
{
	return 0;
}

	}
}
