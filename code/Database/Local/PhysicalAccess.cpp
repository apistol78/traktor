#include <cstring>
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Database/Local/PhysicalAccess.h"
#include "Xml/XmlSerializer.h"
#include "Xml/XmlDeserializer.h"

namespace traktor
{
	namespace db
	{

Path getInstanceObjectPath(const Path& instancePath)
{
	return instancePath.getPathName() + L".xdi";
}

Path getInstanceMetaPath(const Path& instancePath)
{
	return instancePath.getPathName() + L".xdm";
}

Path getInstanceDataPath(const Path& instancePath, const std::wstring& dataName)
{
	return instancePath.getPathName() + L"_" + dataName + L".xdd";
}

Ref< ISerializable > readPhysicalObject(const Path& objectPath)
{
	Ref< IStream > objectStream = FileSystem::getInstance().open(objectPath, File::FmRead);
	if (!objectStream)
		return 0;

	BufferedStream bs(objectStream);

	uint8_t head[5];
	if (bs.read(head, sizeof(head)) != sizeof(head))
	{
		objectStream->close();
		return 0;
	}

	bs.seek(IStream::SeekSet, 0);

	Ref< ISerializable > object;
	if (std::memcmp(head, "<?xml", sizeof(head)) != 0)
		object = BinarySerializer(&bs).readObject();
	else
		object = xml::XmlDeserializer(&bs).readObject();

	objectStream->close();
	return object;
}

bool writePhysicalObject(const Path& objectPath, const ISerializable* object, bool binary)
{
	Ref< IStream > objectStream = FileSystem::getInstance().open(objectPath, File::FmWrite);
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
}
