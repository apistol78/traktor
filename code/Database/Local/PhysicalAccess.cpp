/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
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
		return nullptr;

	// Read head bytes in order to be able to determine serialization type.
	uint8_t head[5];
	if (objectStream->read(head, sizeof(head)) != sizeof(head))
	{
		objectStream->close();
		return nullptr;
	}

	// Rewind to head of stream.
	objectStream->seek(IStream::SeekSet, 0);

	Ref< ISerializable > object;
	if (std::memcmp(head, "<?xml", sizeof(head)) != 0)
		object = BinarySerializer(objectStream).readObject();
	else
		object = xml::XmlDeserializer(objectStream, objectPath.getPathName()).readObject();

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

	if (!result)
		FileSystem::getInstance().remove(objectPath);

	return result;
}

	}
}
