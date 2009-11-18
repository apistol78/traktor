#include "Database/Local/LocalManifest.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.LocalManifest", 0, LocalManifest, ISerializable)

LocalManifest::LocalManifest()
:	m_eventMonitorEnable(false)
,	m_useBinary(false)
{
}

void LocalManifest::setRootGroupPath(const std::wstring& rootGroupPath)
{
	m_rootGroupPath = rootGroupPath;
}

const std::wstring& LocalManifest::getRootGroupPath() const
{
	return m_rootGroupPath;
}

void LocalManifest::setEventMonitorEnable(bool eventMonitorEnable)
{
	m_eventMonitorEnable = eventMonitorEnable;
}

bool LocalManifest::getEventMonitorEnable() const
{
	return m_eventMonitorEnable;
}

void LocalManifest::setEventFile(const std::wstring& eventFile)
{
	m_eventFile = eventFile;
}

const std::wstring& LocalManifest::getEventFile() const
{
	return m_eventFile;
}

void LocalManifest::setUseBinary(bool useBinary)
{
	m_useBinary = useBinary;
}

bool LocalManifest::getUseBinary() const
{
	return m_useBinary;
}

bool LocalManifest::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"rootGroupPath", m_rootGroupPath);
	s >> Member< bool >(L"eventMonitorEnable", m_eventMonitorEnable);
	s >> Member< std::wstring >(L"eventFile", m_eventFile);
	s >> Member< bool >(L"useBinary", m_useBinary);
	return true;
}

	}
}
