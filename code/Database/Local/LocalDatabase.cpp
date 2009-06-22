#include "Database/Local/LocalDatabase.h"
#include "Database/Local/LocalManifest.h"
#include "Database/Local/Context.h"
#include "Database/Local/LocalBus.h"
#include "Database/Local/LocalGroup.h"
#include "Xml/XmlSerializer.h"
#include "Xml/XmlDeserializer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.LocalDatabase", LocalDatabase, IProviderDatabase)

bool LocalDatabase::create(const Path& manifestPath)
{
	std::wstring rootGroupPath = manifestPath.getPathOnly() + L"/Db";

	if (!FileSystem::getInstance().makeDirectory(rootGroupPath))
	{
		log::error << L"Unable to create physical group at \"" << rootGroupPath << L"\"" << Endl;
		return false;
	}

	Ref< LocalManifest > manifest = gc_new< LocalManifest >();

	manifest->setRootGroupPath(rootGroupPath);
	manifest->setEventMonitorEnable(true);
	manifest->setEventFile(rootGroupPath + L"/Events.xvs");
	manifest->setUseBinary(false);

	Ref< Stream > manifestFile = FileSystem::getInstance().open(manifestPath, File::FmWrite);
	if (!manifestFile)
	{
		log::error << L"Unable to create manifest \"" << manifestPath << L"\"" << Endl;
		return false;
	}

	bool result = xml::XmlSerializer(manifestFile).writeObject(manifest);

	manifestFile->close();

	if (!result)
	{
		log::error << L"Unable to write manifest \"" << manifestPath << L"\"" << Endl;
		return false;
	}

	m_context = gc_new< Context >(manifest->getUseBinary());

	if (manifest->getEventMonitorEnable())
		m_bus = gc_new< LocalBus >(manifest->getEventFile());

	m_rootGroup = gc_new< LocalGroup >(m_context, rootGroupPath);

	return true;
}

bool LocalDatabase::open(const Path& manifestPath)
{
	Ref< Stream > manifestFile = FileSystem::getInstance().open(manifestPath, File::FmRead);
	if (!manifestFile)
	{
		log::error << L"Unable to open manifest \"" << manifestPath << L"\", file missing?" << Endl;
		return false;
	}

	Ref< LocalManifest > manifest = xml::XmlDeserializer(manifestFile).readObject< LocalManifest >();
	
	manifestFile->close();

	if (!manifest)
	{
		log::error << L"Unable to read manifest \"" << manifestPath << L"\", possibly corrupted" << Endl;
		return false;
	}

	m_context = gc_new< Context >(manifest->getUseBinary());

	if (manifest->getEventMonitorEnable())
	{
		Path eventFile = manifest->getEventFile();
		if (!FileSystem::getInstance().makeAllDirectories(eventFile.getPathOnly()))
		{
			log::error << L"Unable to ensure event file directory exist" << Endl;
			return false;
		}
		m_bus = gc_new< LocalBus >(eventFile);
	}

	if (!FileSystem::getInstance().makeAllDirectories(manifest->getRootGroupPath()))
	{
		log::error << L"Unable to ensure root group directory exist" << Endl;
		return false;
	}
	m_rootGroup = gc_new< LocalGroup >(m_context, manifest->getRootGroupPath());

	return true;
}

void LocalDatabase::close()
{
	if (m_rootGroup)
		m_rootGroup = 0;

	if (m_bus)
	{
		m_bus->close();
		m_bus = 0;
	}

	if (m_context)
		m_context = 0;
}

IProviderBus* LocalDatabase::getBus()
{
	return m_bus;
}

IProviderGroup* LocalDatabase::getRootGroup()
{
	return m_rootGroup;
}

	}
}
