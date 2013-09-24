#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Misc/SafeDestroy.h"
#include "Pipeline/Report/App/ScriptProcessor.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptContext.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Sql/IConnection.h"
#include "Sql/IResultSet.h"
#include "Sql/Sqlite3/ConnectionSqlite3.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

class Output : public Object
{
	T_RTTI_CLASS;

public:
	void print(const std::wstring& str)
	{
		m_ss << str;
	}

	void printLn(const std::wstring& str)
	{
		m_ss << str << Endl;
	}

	void printSection(int32_t id)
	{
		T_ASSERT (id >= 0 && id < int32_t(m_sections.size()));
		m_ss << m_sections[id];
	}

	int32_t addSection(const std::wstring& section)
	{
		m_sections.push_back(section);
		return int32_t(m_sections.size()) - 1;
	}

	std::wstring getProduct()
	{
		return m_ss.str();
	}

private:
	std::vector< std::wstring > m_sections;
	StringOutputStream m_ss;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.Output", Output, Object)

class Connection : public Object
{
	T_RTTI_CLASS;

public:
	static Ref< sql::IConnection > connect(const std::wstring& cs)
	{
		Ref< sql::ConnectionSqlite3 > connection = new sql::ConnectionSqlite3();
		if (connection->connect(cs))
			return connection;
		else
			return 0;
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.Connection", Connection, Object)

Path FileSystem_getAbsolutePath_1(FileSystem* fileSystem, const Path& relativePath)
{
	return fileSystem->getAbsolutePath(relativePath);
}

Path FileSystem_getAbsolutePath_2(FileSystem* fileSystem, const Path& basePath, const Path& relativePath)
{
	return fileSystem->getAbsolutePath(basePath, relativePath);
}

Ref< Path > FileSystem_getRelativePath(FileSystem* fileSystem, const Path& absolutePath, const Path& relativeToPath)
{
	Path relativePath;
	if (fileSystem->getRelativePath(absolutePath, relativeToPath, relativePath))
		return new Path(relativePath);
	else
		return 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.ScriptProcessor", ScriptProcessor, Object)

bool ScriptProcessor::create()
{
	m_scriptManager = new script::ScriptManagerLua();

	Ref< script::AutoScriptClass< Output > > classOutput = new script::AutoScriptClass< Output >();
	classOutput->addMethod("print", &Output::print);
	classOutput->addMethod("printLn", &Output::printLn);
	classOutput->addMethod("printSection", &Output::printSection);
	m_scriptManager->registerClass(classOutput);

	Ref< script::AutoScriptClass< Connection > > classConnection = new script::AutoScriptClass< Connection >();
	classConnection->addStaticMethod("connect", &Connection::connect);
	m_scriptManager->registerClass(classConnection);

	Ref< script::AutoScriptClass< Path > > classPath = new script::AutoScriptClass< Path >();
	classPath->addConstructor();
	classPath->addConstructor< const std::wstring& >();
	classPath->addMethod("getOriginal", &Path::getOriginal);
	classPath->addMethod("hasVolume", &Path::hasVolume);
	classPath->addMethod("getVolume", &Path::getVolume);
	classPath->addMethod("isRelative", &Path::isRelative);
	classPath->addMethod("getFileName", &Path::getFileName);
	classPath->addMethod("getFileNameNoExtension", &Path::getFileNameNoExtension);
	classPath->addMethod("getPathOnly", &Path::getPathOnly);
	classPath->addMethod("getPathOnlyNoVolume", &Path::getPathOnlyNoVolume);
	classPath->addMethod("getPathName", &Path::getPathName);
	classPath->addMethod("getPathNameNoExtension", &Path::getPathNameNoExtension);
	classPath->addMethod("getPathNameNoVolume", &Path::getPathNameNoVolume);
	classPath->addMethod("getExtension", &Path::getExtension);
	classPath->addMethod("normalized", &Path::normalized);
	m_scriptManager->registerClass(classPath);

	Ref< script::AutoScriptClass< FileSystem > > classFileSystem = new script::AutoScriptClass< FileSystem >();
	classFileSystem->addMethod("exist", &FileSystem::exist);
	classFileSystem->addMethod("remove", &FileSystem::remove);
	classFileSystem->addMethod("makeDirectory", &FileSystem::makeDirectory);
	classFileSystem->addMethod("makeAllDirectories", &FileSystem::makeAllDirectories);
	classFileSystem->addMethod("removeDirectory", &FileSystem::removeDirectory);
	classFileSystem->addMethod("renameDirectory", &FileSystem::renameDirectory);
	classFileSystem->addMethod("getAbsolutePath", &FileSystem_getAbsolutePath_1);
	classFileSystem->addMethod("getAbsolutePath", &FileSystem_getAbsolutePath_2);
	classFileSystem->addMethod("getRelativePath", &FileSystem_getRelativePath);
	m_scriptManager->registerClass(classFileSystem);

	Ref< script::AutoScriptClass< sql::IResultSet > > classIResultSet = new script::AutoScriptClass< sql::IResultSet >();
	classIResultSet->addMethod("next", &sql::IResultSet::next);
	classIResultSet->addMethod("getColumnCount", &sql::IResultSet::getColumnCount);
	classIResultSet->addMethod("getColumnName", &sql::IResultSet::getColumnName);
	classIResultSet->addMethod< int32_t, int32_t >("getInt32", &sql::IResultSet::getInt32);
	//classIResultSet->addMethod< int64_t, int32_t >("getInt64", &sql::IResultSet::getInt64);
	classIResultSet->addMethod< float, int32_t >("getFloat", &sql::IResultSet::getFloat);
	//classIResultSet->addMethod< double, int32_t >("getDouble", &sql::IResultSet::getDouble);
	classIResultSet->addMethod< std::wstring, int32_t >("getString", &sql::IResultSet::getString);
	classIResultSet->addMethod("findColumn", &sql::IResultSet::findColumn);
	classIResultSet->addMethod< int32_t, const std::wstring& >("getInt32ByName", &sql::IResultSet::getInt32);
	//classIResultSet->addMethod< int64_t, const std::wstring& >("getInt64ByName", &sql::IResultSet::getInt64);
	classIResultSet->addMethod< float, const std::wstring& >("getFloatByName", &sql::IResultSet::getFloat);
	//classIResultSet->addMethod< double, const std::wstring& >("getDoubleByName", &sql::IResultSet::getDouble);
	classIResultSet->addMethod< std::wstring, const std::wstring& >("getStringByName", &sql::IResultSet::getString);
	m_scriptManager->registerClass(classIResultSet);

	Ref< script::AutoScriptClass< sql::IConnection > > classIConnection = new script::AutoScriptClass< sql::IConnection >();
	classIConnection->addMethod("connect", &sql::IConnection::connect);
	classIConnection->addMethod("disconnect", &sql::IConnection::disconnect);
	classIConnection->addMethod("executeQuery", &sql::IConnection::executeQuery);
	classIConnection->addMethod("executeUpdate", &sql::IConnection::executeUpdate);
	classIConnection->addMethod("lastInsertId", &sql::IConnection::lastInsertId);
	classIConnection->addMethod("tableExists", &sql::IConnection::tableExists);
	m_scriptManager->registerClass(classIConnection);

	return true;
}

void ScriptProcessor::destroy()
{
	safeDestroy(m_scriptManager);
}

bool ScriptProcessor::generateFromFile(const std::wstring& fileName, std::wstring& output) const
{
	Ref< IStream > file = FileSystem::getInstance().open(fileName, traktor::File::FmRead);
	if (!file)
		return false;

	Utf8Encoding encoding;
	BufferedStream stream(file);
	StringReader reader(&stream, &encoding);
	StringOutputStream ss;

	std::wstring tmp;
	while (reader.readLine(tmp) >= 0)
		ss << tmp << Endl;

	file->close();

	return generateFromSource(ss.str(), output);
}

bool ScriptProcessor::generateFromSource(const std::wstring& source, std::wstring& output) const
{
	StringOutputStream ss;
	size_t offset = 0;

	Ref< Output > o = new Output();

	ss << L"function __main__()" << Endl;

	for (;;)
	{
		size_t s = source.find(L"<?--", offset);
		if (s == source.npos)
			break;

		size_t e = source.find(L"--?>", s);
		if (e == source.npos)
			return false;

		int32_t id = o->addSection(source.substr(offset, s - offset));
		ss << L"\toutput:printSection(" << id << L")" << Endl;
		ss << source.substr(s + 5, e - s - 5) << Endl;

		offset = e + 4;
	}

	int32_t id = o->addSection(source.substr(offset));
	ss << L"\toutput:printSection(" << id << L")" << Endl;
	ss << L"end" << Endl;

	Ref< script::IScriptResource > scriptResource = m_scriptManager->compile(L"", ss.str(), 0, 0);
	if (!scriptResource)
		return false;

	Ref< script::IScriptContext > scriptContext = m_scriptManager->createContext(scriptResource, 0);
	if (!scriptContext)
		return false;

	scriptContext->setGlobal("output", script::Any::fromObject(o));
	scriptContext->setGlobal("fileSystem", script::Any::fromObject(&FileSystem::getInstance()));
	scriptContext->executeFunction("__main__");

	output = o->getProduct();
	return true;
}

	}
}
