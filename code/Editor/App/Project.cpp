#include "Editor/App/Project.h"
#include "Editor/Settings.h"
#include "Database/Local/LocalDatabase.h"
#include "Database/Compact/CompactDatabase.h"
#include "Database/Database.h"
#include "Xml/XmlDeserializer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

db::Database* openDatabase(const std::wstring& databaseName, bool create)
{
	Ref< db::IProviderDatabase > providerDatabase;

	if (endsWith(toLower(databaseName), L".manifest"))
	{
		Ref< db::LocalDatabase > localDatabase = gc_new< db::LocalDatabase >();
		if (!localDatabase->open(databaseName))
		{
			if (!create || !localDatabase->create(databaseName))
				return 0;
		}

		providerDatabase = localDatabase;
	}
	else if (endsWith(toLower(databaseName), L".compact"))
	{
		Ref< db::CompactDatabase > compactDatabase = gc_new< db::CompactDatabase >();
		if (!compactDatabase->open(databaseName))
		{
			if (!create || !compactDatabase->create(databaseName))
				return 0;
		}

		providerDatabase = compactDatabase;
	}

	T_ASSERT (providerDatabase);

	Ref< db::Database > database = gc_new< db::Database >();
	return database->create(providerDatabase) ? database.getPtr() : 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.Project", Project, IProject)

bool Project::create(const Path& fileName)
{
	return false;
}

bool Project::open(const Path& fileName)
{
	Ref< Stream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
		return false;

	Ref< PropertyGroup > group = dynamic_type_cast< PropertyGroup* >(
		xml::XmlDeserializer(file).readObject()
	);
	file->close();

	if (!group)
		return false;

	m_settings = gc_new< Settings >(
		group,
		gc_new< PropertyGroup >()
	);

	std::wstring sourceManifest = m_settings->getProperty< editor::PropertyString >(L"Project.SourceManifest");
	std::wstring outputManifest = m_settings->getProperty< editor::PropertyString >(L"Project.OutputManifest");

	m_sourceDatabase = openDatabase(sourceManifest, false);
	m_outputDatabase = openDatabase(outputManifest, false);

	if (!m_sourceDatabase || !m_outputDatabase)
		return false;

	return true;
}

void Project::close()
{
	if (m_settings)
		m_settings = 0;

	if (m_sourceDatabase)
	{
		m_sourceDatabase->close();
		m_sourceDatabase = 0;
	}

	if (m_outputDatabase)
	{
		m_outputDatabase->close();
		m_outputDatabase = 0;
	}
}

Settings* Project::getSettings()
{
	return m_settings;
}

db::Database* Project::getSourceDatabase()
{
	return m_sourceDatabase;
}

db::Database* Project::getOutputDatabase()
{
	return m_outputDatabase;
}

	}
}
