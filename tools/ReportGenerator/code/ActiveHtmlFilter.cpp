#include <Core/Io/StringOutputStream.h>
#include <Core/Misc/WildCompare.h>
#include <Script/AutoScriptClass.h>
#include <Script/IScriptContext.h>
#include <Script/Js/ScriptManagerJs.h>
#include <Sql/IConnection.h>
#include <Sql/IResultSet.h>
#include <Sql/Sqlite3/ConnectionSqlite3.h>
#include "ActiveHtmlFilter.h"

using namespace traktor;

namespace
{

const wchar_t* c_scriptContextErrorPage = L"<html><body>Unable to create script context</body></html>";
const wchar_t* c_scriptExecuteErrorPage = L"<html><body>Unable to execute script</body></html>";
const wchar_t* c_parseErrorPage = L"<html><body>Unable to parse source</body></html>";

class Global : public Object
{
	T_RTTI_CLASS;

public:
	Global(StringOutputStream& ss)
	:	m_ss(ss)
	{
	}

	Ref< sql::IConnection > connect(const std::wstring& connectionString)
	{
		Ref< sql::ConnectionSqlite3 > connection = new sql::ConnectionSqlite3();
		if (connection->connect(connectionString))
			return connection;
		else
			return 0;
	}

	void print(const std::wstring& str)
	{
		m_ss << str;
	}

	void printLn(const std::wstring& str)
	{
		m_ss << str << Endl;
	}

private:
	StringOutputStream& m_ss;
};

T_IMPLEMENT_RTTI_CLASS(L"Global", Global, Object)

}

T_IMPLEMENT_RTTI_CLASS(L"ActiveHtmlFilter", ActiveHtmlFilter, Object)

bool ActiveHtmlFilter::create()
{
	m_scriptManager = new script::ScriptManagerJs();

	Ref< script::AutoScriptClass< Global > > globalClass = new script::AutoScriptClass< Global >();
	globalClass->addMethod(L"connect", &Global::connect);
	globalClass->addMethod(L"print", &Global::print);
	globalClass->addMethod(L"printLn", &Global::printLn);
	m_scriptManager->registerClass(globalClass);

	Ref< script::AutoScriptClass< sql::IResultSet > > sqlIResultSetClass = new script::AutoScriptClass< sql::IResultSet >();
	sqlIResultSetClass->addMethod(L"next", &sql::IResultSet::next);
	sqlIResultSetClass->addMethod(L"getColumnCount", &sql::IResultSet::getColumnCount);
	sqlIResultSetClass->addMethod(L"getColumnNamei", &sql::IResultSet::getColumnName);

	////sqlIResultSetClass->addMethod(L"getColumnType", &sql::IResultSet::getColumnType);
	sqlIResultSetClass->addMethod< int32_t, int32_t >(L"getInt32", &sql::IResultSet::getInt32);
	sqlIResultSetClass->addMethod< int64_t, int32_t >(L"getInt64", &sql::IResultSet::getInt64);
	sqlIResultSetClass->addMethod< float, int32_t >(L"getFloat", &sql::IResultSet::getFloat);
	sqlIResultSetClass->addMethod< double, int32_t >(L"getDouble", &sql::IResultSet::getDouble);
	sqlIResultSetClass->addMethod< std::wstring, int32_t >(L"getString", &sql::IResultSet::getString);
	sqlIResultSetClass->addMethod< int32_t, const std::wstring& >(L"findColumn", &sql::IResultSet::findColumn);
	sqlIResultSetClass->addMethod< int32_t, const std::wstring& >(L"getInt32ByName", &sql::IResultSet::getInt32);
	sqlIResultSetClass->addMethod< int64_t, const std::wstring& >(L"getInt64ByName", &sql::IResultSet::getInt64);
	sqlIResultSetClass->addMethod< float, const std::wstring& >(L"getFloatByName", &sql::IResultSet::getFloat);
	sqlIResultSetClass->addMethod< double, const std::wstring& >(L"getDoubleByName", &sql::IResultSet::getDouble);
	sqlIResultSetClass->addMethod< std::wstring, const std::wstring& >(L"getStringByName", &sql::IResultSet::getString);

	m_scriptManager->registerClass(sqlIResultSetClass);

	Ref< script::AutoScriptClass< sql::IConnection > > sqlIConnectionClass = new script::AutoScriptClass< sql::IConnection >();
	sqlIConnectionClass->addMethod(L"connect", &sql::IConnection::connect);
	sqlIConnectionClass->addMethod(L"disconnect", &sql::IConnection::disconnect);
	sqlIConnectionClass->addMethod(L"executeQuery", &sql::IConnection::executeQuery);
	sqlIConnectionClass->addMethod(L"executeUpdate", &sql::IConnection::executeUpdate);
	sqlIConnectionClass->addMethod(L"lastInsertId", &sql::IConnection::lastInsertId);
	sqlIConnectionClass->addMethod(L"tableExists", &sql::IConnection::tableExists);
	m_scriptManager->registerClass(sqlIConnectionClass);

	return true;
}

void ActiveHtmlFilter::destroy()
{
	m_scriptManager = 0;
}

std::wstring ActiveHtmlFilter::generate(const std::wstring& source) const
{
	Ref< script::IScriptContext > scriptContext = m_scriptManager->createContext();
	if (!scriptContext)
		return c_scriptContextErrorPage;

	StringOutputStream ss;
	size_t offset = 0;

	Ref< Global > global = new Global(ss);
	scriptContext->setGlobal(L"Global", script::Any(global));

	for (;;)
	{
		size_t s = source.find(L"<?tap", offset);
		if (s == source.npos)
			break;

		size_t e = source.find(L"?>", s);
		if (e == source.npos)
			return c_parseErrorPage;

		ss << source.substr(offset, s - offset);

		std::wstring sc = source.substr(s + 5, e - s - 5);

		Ref< script::IScriptResource > scriptResource = m_scriptManager->compile(sc, false, 0);
		if (!scriptResource)
			return c_scriptExecuteErrorPage;

		if (!scriptContext->executeScript(scriptResource, Guid()))
			return c_scriptExecuteErrorPage;

		offset = e + 2;
	}

	ss << source.substr(offset);

	return ss.str();
}
