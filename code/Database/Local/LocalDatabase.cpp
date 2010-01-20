#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Database/ConnectionString.h"
#include "Database/Local/Context.h"
#include "Database/Local/LocalBus.h"
#include "Database/Local/LocalDatabase.h"
#include "Database/Local/LocalGroup.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.LocalDatabase", 0, LocalDatabase, IProviderDatabase)

bool LocalDatabase::create(const ConnectionString& connectionString)
{
	if (!connectionString.have(L"groupPath"))
		return false;

	std::wstring groupPath = connectionString.get(L"groupPath");

	Path groupPathA = FileSystem::getInstance().getAbsolutePath(groupPath);
	if (!FileSystem::getInstance().makeAllDirectories(groupPathA))
	{
		log::error << L"Unable to create physical group at \"" << groupPath << L"\"" << Endl;
		return false;
	}

	return open(connectionString);
}

bool LocalDatabase::open(const ConnectionString& connectionString)
{
	if (!connectionString.have(L"groupPath"))
		return false;

	std::wstring groupPath = connectionString.get(L"groupPath");
	bool eventFile = connectionString.have(L"eventFile") ? parseString< bool >(connectionString.get(L"eventFile")) : true;
	bool binary = connectionString.have(L"binary") ? parseString< bool >(connectionString.get(L"binary")) : false;

	Path groupPathA = FileSystem::getInstance().getAbsolutePath(groupPath);

	m_context = new Context(binary);

	if (eventFile)
	{
		Path eventPath = groupPathA + L"/Events.shm";
		if (!FileSystem::getInstance().makeAllDirectories(eventPath.getPathOnly()))
		{
			log::error << L"Unable to ensure event file directory exist" << Endl;
			return false;
		}
		m_bus = new LocalBus(eventPath.getPathName());
	}

	if (!FileSystem::getInstance().makeAllDirectories(groupPathA))
	{
		log::error << L"Unable to ensure root group directory exist" << Endl;
		return false;
	}

	m_rootGroup = new LocalGroup(m_context, groupPathA);
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

Ref< IProviderBus > LocalDatabase::getBus()
{
	return m_bus;
}

Ref< IProviderGroup > LocalDatabase::getRootGroup()
{
	return m_rootGroup;
}

	}
}
