#include <algorithm>
#include "Database/Local/FileDataAccess.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/LocalFileLink.h"
#include "Xml/XmlSerializer.h"
#include "Xml/XmlDeserializer.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Misc/StringUtils.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace db
	{
		namespace
		{

inline Path getInstanceObjectPath(const Path& instancePath)
{
	return std::wstring(instancePath) + L".xdi";
}

inline Path getInstanceMetaPath(const Path& instancePath)
{
	return std::wstring(instancePath) + L".xdm";
}

template < typename ObjectType >
Ref< ObjectType > readPhysicalObject(const Path& objectPath)
{
	Ref< Stream > objectStream = FileSystem::getInstance().open(objectPath, File::FmRead);
	if (!objectStream)
		return 0;

	uint8_t head[5];
	if (objectStream->read(head, sizeof(head)) != sizeof(head))
	{
		objectStream->close();
		return 0;
	}

	objectStream->seek(Stream::SeekSet, 0);

	Ref< ObjectType > object;
	if (std::memcmp(head, "<?xml", sizeof(head)) != 0)
		object = BinarySerializer(objectStream).readObject< ObjectType >();
	else
		object = xml::XmlDeserializer(objectStream).readObject< ObjectType >();

	objectStream->close();
	return object;
}

bool writePhysicalObject(const Path& objectPath, const Serializable* object, bool binary)
{
	Ref< Stream > objectStream = FileSystem::getInstance().open(objectPath, File::FmWrite);
	if (!objectStream)
		return false;

	bool result;
	if (binary)
		result = BinarySerializer(objectStream).writeObject(object);
	else
		result = xml::XmlSerializer(objectStream).writeObject(object);

	objectStream->close();
	return result;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.FileDataAccess", FileDataAccess, DataAccess)

FileDataAccess::FileDataAccess(bool binary)
:	m_binary(binary)
{
}

bool FileDataAccess::renameGroup(const Path& groupPath, const std::wstring& newGroupName) const
{
	return FileSystem::getInstance().renameDirectory(groupPath, newGroupName);
}

bool FileDataAccess::removeGroup(const Path& groupPath) const
{
	return FileSystem::getInstance().removeDirectory(groupPath);
}

bool FileDataAccess::createGroup(const Path& groupPath, const std::wstring& newGroupName) const
{
	Path newGroupPath = std::wstring(groupPath) + L"/" + newGroupName;

	if (FileSystem::getInstance().exist(newGroupPath))
		return false;

	return FileSystem::getInstance().makeDirectory(newGroupPath);
}

bool FileDataAccess::enumerateGroups(const Path& groupPath, std::vector< Path >& outGroups) const
{
	RefArray< File > groupFiles;
	if (!FileSystem::getInstance().find(std::wstring(groupPath) + L"/*.*", groupFiles))
		return false;

	for (RefArray< File >::iterator i = groupFiles.begin(); i != groupFiles.end(); ++i)
	{
		const Path& path = (*i)->getPath();
		if ((*i)->isDirectory() && path.getFileName() != L"." && path.getFileName() != L"..")
			outGroups.push_back(path);
		else if (!(*i)->isDirectory() && compareIgnoreCase(path.getExtension(), L"xgl") == 0)
		{
			Ref< LocalFileLink > link = readPhysicalObject< LocalFileLink >(path);
			if (link)
				outGroups.push_back(link->getPath());
		}
	}

	return true;
}

bool FileDataAccess::enumerateInstances(const Path& groupPath, std::vector< Path >& outInstances) const
{
	RefArray< File > groupFiles;
	if (!FileSystem::getInstance().find(std::wstring(groupPath) + L"/*.*", groupFiles))
		return false;

	for (RefArray< File >::iterator i = groupFiles.begin(); i != groupFiles.end(); ++i)
	{
		const Path& path = (*i)->getPath();
		if (compareIgnoreCase(path.getExtension(), L"xdm") == 0)
			outInstances.push_back(path);
		else if (compareIgnoreCase(path.getExtension(), L"xil") == 0)
		{
			Ref< LocalFileLink > link = readPhysicalObject< LocalFileLink >(path);
			if (link)
				outInstances.push_back(link->getPath());
		}
	}

	return true;
}

Ref< LocalInstanceMeta > FileDataAccess::getInstanceMeta(const Path& instancePath) const
{
	return readPhysicalObject< LocalInstanceMeta >(getInstanceMetaPath(instancePath));
}

bool FileDataAccess::createInstance(const Path& instancePath, const Guid& instanceGuid) const
{
	if (
		FileSystem::getInstance().exist(getInstanceObjectPath(instancePath)) ||
		FileSystem::getInstance().exist(getInstanceMetaPath(instancePath))
	)
		return false;

	Ref< LocalInstanceMeta > instanceMeta = gc_new< LocalInstanceMeta >();
	instanceMeta->setGuid(instanceGuid);

	return writePhysicalObject(getInstanceMetaPath(instancePath), instanceMeta, m_binary);
}

bool FileDataAccess::renameInstance(const Path& instancePath, const std::wstring& newInstanceName) const
{
	Path newInstancePath = instancePath.getPathOnly() + L"/" + newInstanceName;

	if (!FileSystem::getInstance().move(getInstanceObjectPath(newInstancePath), getInstanceObjectPath(instancePath), true))
		return false;
	if (!FileSystem::getInstance().move(getInstanceMetaPath(newInstancePath), getInstanceMetaPath(instancePath), true))
		return false;

	return true;
}

bool FileDataAccess::removeInstance(const Path& instancePath) const
{
	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(getInstanceMetaPath(instancePath));
	if (!instanceMeta)
		return false;

	const std::vector< std::wstring >& blobs = instanceMeta->getBlobs();
	for (std::vector< std::wstring >::const_iterator i = blobs.begin(); i != blobs.end(); ++i)
	{
		Path instanceBlobPath = instancePath.getPathOnly() + L"/" + instancePath.getFileNameNoExtension() + L"_" + *i + L".xdd";
		if (!FileSystem::getInstance().remove(instanceBlobPath))
			log::warning << L"Unable to remove blob \"" << *i << L"\"" << Endl;
	}

	bool result = true;

	if (!FileSystem::getInstance().remove(getInstanceMetaPath(instancePath)))
		result = false;
	if (!FileSystem::getInstance().remove(getInstanceObjectPath(instancePath)))
		result = false;

	return result;
}

Ref< Serializable > FileDataAccess::readObject(const Path& instancePath) const
{
	return readPhysicalObject< Serializable >(getInstanceObjectPath(instancePath));
}

bool FileDataAccess::writeObject(const Path& instancePath, Serializable* object) const
{
	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(getInstanceMetaPath(instancePath));
	if (!instanceMeta)
		return 0;

	std::wstring primaryTypeName = type_name(object);

	if (instanceMeta->getPrimaryType() != primaryTypeName)
	{
		instanceMeta->setPrimaryType(primaryTypeName);
		if (!writePhysicalObject(getInstanceMetaPath(instancePath), instanceMeta, m_binary))
			return 0;
	}

	return writePhysicalObject(getInstanceObjectPath(instancePath), object, m_binary);
}

uint32_t FileDataAccess::enumerateDataNames(const Path& instancePath, std::vector< std::wstring >& outDataNames) const
{
	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(getInstanceMetaPath(instancePath));
	if (!instanceMeta)
		return 0;

	outDataNames = instanceMeta->getBlobs();
	return uint32_t(outDataNames.size());
}

Ref< Stream > FileDataAccess::readData(const Path& instancePath, const std::wstring& dataName) const
{
	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(getInstanceMetaPath(instancePath));
	if (!instanceMeta)
		return 0;

	const std::vector< std::wstring >& blobs = instanceMeta->getBlobs();
	if (std::find(blobs.begin(), blobs.end(), dataName) == blobs.end())
		return 0;

	Path instanceBlobPath = instancePath.getPathOnly() + L"/" + instancePath.getFileNameNoExtension() + L"_" + dataName + L".xdd";

	Ref< Stream > stream = FileSystem::getInstance().open(instanceBlobPath, File::FmRead);
	if (!stream)
		return 0;

	return gc_new< BufferedStream >(stream);
}

Ref< Stream > FileDataAccess::writeData(const Path& instancePath, const std::wstring& dataName) const
{
	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(getInstanceMetaPath(instancePath));
	if (!instanceMeta)
		return 0;

	const std::vector< std::wstring >& blobs = instanceMeta->getBlobs();
	if (std::find(blobs.begin(), blobs.end(), dataName) == blobs.end())
	{
		instanceMeta->addBlob(dataName);
		if (!writePhysicalObject(getInstanceMetaPath(instancePath), instanceMeta, m_binary))
			return 0;
	}

	Path instanceBlobPath = instancePath.getPathOnly() + L"/" + instancePath.getFileNameNoExtension() + L"_" + dataName + L".xdd";

	return FileSystem::getInstance().open(instanceBlobPath, File::FmWrite);
}

	}
}
