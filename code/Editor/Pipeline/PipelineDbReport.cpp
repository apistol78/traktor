#include "Core/Io/StringOutputStream.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Editor/Pipeline/PipelineDbReport.h"
#include "Sql/IConnection.h"
#include "Sql/IResultSet.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

const int32_t c_version = 1;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDbReport", PipelineDbReport, IPipelineReport)

PipelineDbReport::PipelineDbReport(Semaphore& lock, sql::IConnection* connection, const std::wstring& table, const Guid& guid)
:	m_lock(lock)
,	m_connection(connection)
,	m_table(table)
,	m_guid(guid)
{
}

PipelineDbReport::~PipelineDbReport()
{
	T_EXCEPTION_GUARD_BEGIN;
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	Ref< sql::IResultSet > rs;
	StringOutputStream ss;

	if (m_ivalues.empty() && m_svalues.empty())
		return;

	bool haveTable = m_connection->tableExists(m_table);

	// Ensure table version.
	if (haveTable)
	{
		haveTable = false;
		if (m_connection->tableExists(m_table + L"_Version"))
		{
			Ref< sql::IResultSet > rs;
			rs = m_connection->executeQuery(L"select * from " + m_table + L"_Version");
			if (rs && rs->next())
			{
				if (rs->getInt32(0) == c_version)
					haveTable = true;
			}
		}

		// Drop table if incorrect version.
		if (!haveTable)
		{
			m_connection->executeUpdate(L"drop table " + m_table);
			m_connection->executeUpdate(L"drop table " + m_table + L"_Version");
		}
	}

	// Create table if it doesn't exist.
	if (!haveTable)
	{
		ss <<
			L"create table " << m_table << L" (" <<
			L"id integer primary key, " <<
			L"guid char(37)";
		for (std::map< std::wstring, int32_t >::const_iterator i = m_ivalues.begin(); i != m_ivalues.end(); ++i)
			ss << L", " << i->first << L" integer";
		for (std::map< std::wstring, std::wstring >::const_iterator i = m_svalues.begin(); i != m_svalues.end(); ++i)
			ss << L", " << i->first << L" varchar";
		ss << L")";
		m_connection->executeUpdate(ss.str());
		m_connection->executeUpdate(L"create table " + m_table + L"_Version (major integer)");
		m_connection->executeUpdate(L"insert into " + m_table + L"_Version (major) values (" + toString(c_version) + L")");
	}
	// Remove report if it already exists.
	else
	{
		ss << L"delete from " << m_table << L" where guid='" << m_guid.format() << L"'";
		m_connection->executeUpdate(ss.str());
	}

	// Insert report record.
	ss.reset();
	ss << L"insert into " << m_table << L" (guid";
	for (std::map< std::wstring, int32_t >::const_iterator i = m_ivalues.begin(); i != m_ivalues.end(); ++i)
		ss << L", " << i->first;
	for (std::map< std::wstring, std::wstring >::const_iterator i = m_svalues.begin(); i != m_svalues.end(); ++i)
		ss << L", " << i->first;
	ss << L") values ('" << m_guid.format() << L"'";
	for (std::map< std::wstring, int32_t >::const_iterator i = m_ivalues.begin(); i != m_ivalues.end(); ++i)
		ss << L", " << i->second;
	for (std::map< std::wstring, std::wstring >::const_iterator i = m_svalues.begin(); i != m_svalues.end(); ++i)
		ss << L", '" << i->second << L"'";
	ss << L")";
	m_connection->executeUpdate(ss.str());

	T_EXCEPTION_GUARD_END;
}

void PipelineDbReport::set(const std::wstring& name, int32_t value)
{
	m_ivalues.insert(std::make_pair(name, value));
}

void PipelineDbReport::set(const std::wstring& name, const std::wstring& value)
{
	m_svalues.insert(std::make_pair(name, value));
}

	}
}
