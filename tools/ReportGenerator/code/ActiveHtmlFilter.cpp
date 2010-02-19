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
	globalClass->addMethod< TypeList< Ref< sql::IConnection >, const std::wstring& > >(L"connect", &Global::connect);
	globalClass->addMethod< TypeList< void, const std::wstring& > >(L"print", &Global::print);
	globalClass->addMethod< TypeList< void, const std::wstring& > >(L"printLn", &Global::printLn);
	m_scriptManager->registerClass(globalClass);

	Ref< script::AutoScriptClass< sql::IResultSet > > sqlIResultSetClass = new script::AutoScriptClass< sql::IResultSet >();
	sqlIResultSetClass->addMethod< TypeList< bool > >(L"next", &sql::IResultSet::next);
	sqlIResultSetClass->addConstMethod< TypeList< int32_t > >(L"getColumnCount", &sql::IResultSet::getColumnCount);
	sqlIResultSetClass->addConstMethod< TypeList< std::wstring, int32_t > >(L"getColumnNamei", &sql::IResultSet::getColumnName);
	//sqlIResultSetClass->addConstMethod< TypeList< int32_t, int32_t > >(L"getColumnType", &sql::IResultSet::getColumnType);
	sqlIResultSetClass->addConstMethod< TypeList< int32_t, int32_t > >(L"getInt32", &sql::IResultSet::getInt32);
	//sqlIResultSetClass->addConstMethod< TypeList< int64_t, int32_t > >(L"getInt64", &sql::IResultSet::getInt64);
	sqlIResultSetClass->addConstMethod< TypeList< float, int32_t > >(L"getFloat", &sql::IResultSet::getFloat);
	//sqlIResultSetClass->addConstMethod< TypeList< double, int32_t > >(L"getDouble", &sql::IResultSet::getDouble);
	sqlIResultSetClass->addConstMethod< TypeList< std::wstring, int32_t > >(L"getString", &sql::IResultSet::getString);
	sqlIResultSetClass->addConstMethod< TypeList< int32_t, const std::wstring& > >(L"findColumn", &sql::IResultSet::findColumn);
	sqlIResultSetClass->addConstMethod< TypeList< int32_t, const std::wstring& > >(L"getInt32ByName", &sql::IResultSet::getInt32);
	//sqlIResultSetClass->addConstMethod< TypeList< int64_t, const std::wstring& > >(L"getInt64ByName", &sql::IResultSet::getInt64);
	sqlIResultSetClass->addConstMethod< TypeList< float, const std::wstring& > >(L"getFloatByName", &sql::IResultSet::getFloat);
	//sqlIResultSetClass->addConstMethod< TypeList< double, const std::wstring& > >(L"getDoubleByName", &sql::IResultSet::getDouble);
	sqlIResultSetClass->addConstMethod< TypeList< std::wstring, const std::wstring& > >(L"getStringByName", &sql::IResultSet::getString);
	m_scriptManager->registerClass(sqlIResultSetClass);

	Ref< script::AutoScriptClass< sql::IConnection > > sqlIConnectionClass = new script::AutoScriptClass< sql::IConnection >();
	sqlIConnectionClass->addMethod< TypeList< bool, const std::wstring& > >(L"connect", &sql::IConnection::connect);
	sqlIConnectionClass->addMethod< TypeList< void > >(L"disconnect", &sql::IConnection::disconnect);
	sqlIConnectionClass->addMethod< TypeList< Ref< sql::IResultSet >, const std::wstring& > >(L"executeQuery", &sql::IConnection::executeQuery);
	sqlIConnectionClass->addMethod< TypeList< int32_t, const std::wstring& > >(L"executeUpdate", &sql::IConnection::executeUpdate);
	sqlIConnectionClass->addMethod< TypeList< int32_t > >(L"lastInsertId", &sql::IConnection::lastInsertId);
	sqlIConnectionClass->addMethod< TypeList< bool, const std::wstring& > >(L"tableExists", &sql::IConnection::tableExists);
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

		if (!scriptContext->executeScript(sc, false, 0))
			return c_scriptExecuteErrorPage;

		offset = e + 2;
	}

	ss << source.substr(offset);

	return ss.str();
}
