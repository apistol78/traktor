#include "Core/Io/StringOutputStream.h"
#include "Editor/Pipeline/PipelineDbReport.h"
#include "Sql/IConnection.h"
#include "Sql/IResultSet.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDbReport", PipelineDbReport, IPipelineReport)

PipelineDbReport::PipelineDbReport(sql::IConnection* connection, const std::wstring& table, const Guid& guid)
:	m_connection(connection)
,	m_table(table)
,	m_guid(guid)
{
}

PipelineDbReport::~PipelineDbReport()
{
	T_EXCEPTION_GUARD_BEGIN;

	Ref< sql::IResultSet > rs;
	StringOutputStream ss;

	if (m_values.empty())
		return;

	// Create table if it doesn't exist.
	if (!m_connection->tableExists(m_table))
	{
		ss <<
			L"create table " << m_table << L" (" <<
			L"id integer primary key, " <<
			L"guid char(37)";
		for (std::map< std::wstring, int32_t >::const_iterator i = m_values.begin(); i != m_values.end(); ++i)
			ss << L", " << i->first << L" integer";
		ss << L")";
		m_connection->executeUpdate(ss.str());
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
	for (std::map< std::wstring, int32_t >::const_iterator i = m_values.begin(); i != m_values.end(); ++i)
		ss << L", " << i->first;
	ss << L") values ('" << m_guid.format() << L"'";
	for (std::map< std::wstring, int32_t >::const_iterator i = m_values.begin(); i != m_values.end(); ++i)
		ss << L", " << i->second;
	ss << L")";
	m_connection->executeUpdate(ss.str());

	T_EXCEPTION_GUARD_END;
}

void PipelineDbReport::set(const std::wstring& name, int32_t value)
{
	m_values.insert(std::make_pair(name, value));
}

	}
}
