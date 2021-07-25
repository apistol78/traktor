#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Thread/Acquire.h"
#include "Editor/Pipeline/PipelineDbSql.h"
#include "Sql/IResultSet.h"
#include "Sql/Sqlite3/ConnectionSqlite3.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

const int32_t c_version = 7;

int32_t hash(const std::wstring& s)
{
	Adler32 a;
	a.begin();
	a.feed(s.c_str(), s.length() * sizeof(wchar_t));
	a.end();
	return a.get();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDbSql", PipelineDbSql, IPipelineDb)

PipelineDbSql::PipelineDbSql()
:	m_transaction(false)
{
}

bool PipelineDbSql::open(const std::wstring& connectionString)
{
	Ref< sql::ConnectionSqlite3 > connection = new sql::ConnectionSqlite3();
	if (!connection->connect(connectionString))
		return false;

	bool haveTable = connection->tableExists(L"PipelineHash");

	// Ensure db latest version.
	if (haveTable)
	{
		haveTable = false;

		if (connection->tableExists(L"Version"))
		{
			Ref< sql::IResultSet > rs;
			rs = connection->executeQuery(L"select * from Version");
			if (rs && rs->next())
			{
				if (rs->getInt32(0) == c_version)
					haveTable = true;
			}
		}

		// Drop all tables if incorrect version.
		if (!haveTable)
		{
			connection->executeUpdate(L"drop table PipelineHash");
			connection->executeUpdate(L"drop table PipelineFile");
			connection->executeUpdate(L"drop table Version");
		}
	}

	// Create tables if they doesn't exist.
	if (!haveTable)
	{
		if (connection->executeUpdate(
			L"create table PipelineHash ("
			L"guid char(37) primary key,"
			L"pipelineHash integer,"
			L"sourceAssetHash integer,"
			L"sourceDataHash integer,"
			L"filesHash integer"
			L")"
		) < 0)
			return false;

		if (connection->executeUpdate(
			L"create table PipelineFile ("
			L"pathHash integer primary key,"
			L"path varchar(1024),"
			L"size integer,"
			L"lastWriteTime integer,"
			L"hash integer"
			L")"
		) < 0)
			return false;

		if (connection->executeUpdate(L"create table Version (major integer)") < 0)
			return false;

		if (connection->executeUpdate(L"insert into Version (major) values (" + toString(c_version) + L")") <= 0)
			return false;
	}

	m_connection = connection;
	return true;
}

void PipelineDbSql::close()
{
	if (m_connection)
	{
		m_connection->disconnect();
		m_connection = nullptr;
	}
}

void PipelineDbSql::beginTransaction()
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireWriter)(m_lock);
	m_transaction = false;
}

void PipelineDbSql::endTransaction()
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireWriter)(m_lock);
	if (m_transaction)
	{
		m_connection->executeUpdate(L"commit transaction");
		m_transaction = false;
	}
}

void PipelineDbSql::setDependency(const Guid& guid, const PipelineDependencyHash& hash)
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireWriter)(m_lock);

	if (!m_transaction)
	{
		m_connection->executeUpdate(L"begin transaction");
		m_transaction = true;
	}

	Ref< sql::IResultSet > rs;

	m_ss.reset();
	m_ss <<
		L"insert or replace into PipelineHash (guid, pipelineHash, sourceAssetHash, sourceDataHash, filesHash) "
		L"values (" <<
		L"'" << guid.format() << L"'," <<
		hash.pipelineHash << L"," <<
		hash.sourceAssetHash << L"," <<
		hash.sourceDataHash << L"," <<
		hash.filesHash <<
		L")";
	if (m_connection->executeUpdate(m_ss.str()) != 1)
		log::warning << L"Unable to update pipeline hash in database" << Endl;
}

bool PipelineDbSql::getDependency(const Guid& guid, PipelineDependencyHash& outHash) const
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lock);

	Ref< sql::IResultSet > rs;

	// Get hash record.
	m_ss.reset();
	m_ss << L"select * from PipelineHash where guid='" << guid.format() << L"'";
	rs = m_connection->executeQuery(m_ss.str());
	if (!rs || !rs->next())
		return false;

	outHash.pipelineHash = rs->getInt32(L"pipelineHash");
	outHash.sourceAssetHash = rs->getInt32(L"sourceAssetHash");
	outHash.sourceDataHash = rs->getInt32(L"sourceDataHash");
	outHash.filesHash = rs->getInt32(L"filesHash");

	return true;
}

void PipelineDbSql::setFile(const Path& path, const PipelineFileHash& file)
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireWriter)(m_lock);

	if (!m_transaction)
	{
		m_connection->executeUpdate(L"begin transaction");
		m_transaction = true;
	}

	std::wstring pn = toLower(path.normalized().getPathName());
	uint32_t pnh = hash(pn);

	Ref< sql::IResultSet > rs;

	m_ss.reset();
	m_ss <<
		L"insert or replace into PipelineFile (pathHash, path, size, lastWriteTime, hash) "
		L"values (" <<
		pnh << L"," <<
		L"'" << pn << L"'," <<
		file.size << L"," <<
		file.lastWriteTime.getSecondsSinceEpoch() << L"," <<
		file.hash <<
		L")";
	if (m_connection->executeUpdate(m_ss.str()) != 1)
		log::error << L"Unable to update pipeline file hash in database" << Endl;
}

bool PipelineDbSql::getFile(const Path& path, PipelineFileHash& outFile) const
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lock);

	std::wstring pn = toLower(path.normalized().getPathName());
	uint32_t pnh = hash(pn);

	Ref< sql::IResultSet > rs;

	// Get file record.
	m_ss.reset();
	m_ss << L"select * from PipelineFile where pathHash=" << pnh << L" and path='" << pn << L"'";
	rs = m_connection->executeQuery(m_ss.str());
	if (!rs || !rs->next())
		return false;

	outFile.size = rs->getInt64(L"size"),
	outFile.lastWriteTime = DateTime(rs->getInt64(L"lastWriteTime"));
	outFile.hash = rs->getInt32(L"hash");

	return true;
}

uint32_t PipelineDbSql::getDependencyCount() const
{
	return 0;
}

bool PipelineDbSql::getDependencyByIndex(uint32_t index, Guid& outGuid, PipelineDependencyHash& outHash) const
{
	return false;
}

uint32_t PipelineDbSql::getFileCount() const
{
	return 0;
}

bool PipelineDbSql::getFileByIndex(uint32_t index, Path& outPath, PipelineFileHash& outFile) const
{
	return false;
}

	}
}
