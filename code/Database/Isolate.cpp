/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/Reader.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/Writer.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Isolate.h"

namespace traktor::db
{

bool Isolate::createIsolatedInstance(Instance* instance, IStream* stream)
{
	T_ASSERT(stream && stream->canWrite());

	Writer writer(stream);

	writer << instance->getName();
	writer << instance->getGuid().format();

	std::vector< std::wstring > dataNames;
	writer << uint32_t(instance->getDataNames(dataNames));

	Ref< ISerializable > object = instance->getObject();
	if (!BinarySerializer(stream).writeObject(object))
		return false;

	for (const auto& dataName : dataNames)
	{
		writer << dataName;

		Ref< IStream > dataStream = instance->readData(dataName);
		T_ASSERT(dataStream);

		int32_t dataSize = (int32_t)dataStream->available();
		writer << dataSize;

		if (!StreamCopy(stream, dataStream).execute(dataSize))
			return false;
	}

	return true;
}

Ref< Instance > Isolate::createInstanceFromIsolation(Group* group, IStream* stream)
{
	T_ASSERT(stream && stream->canRead());

	Reader reader(stream);

	std::wstring instanceName, instanceGuid;
	reader >> instanceName;
	reader >> instanceGuid;

	uint32_t dataNames;
	reader >> dataNames;

	Guid guid(instanceGuid);
	if (!guid.isValid())
		return nullptr;

	Ref< Instance > instance = group->createInstance(instanceName, CifReplaceExisting, &guid);
	if (!instance)
		return nullptr;

	Ref< ISerializable > object = BinarySerializer(stream).readObject();
	if (!instance->setObject(object))
		return nullptr;

	for (uint32_t i = 0; i < dataNames; ++i)
	{
		std::wstring dataName;
		reader >> dataName;

		int32_t dataSize;
		reader >> dataSize;

		Ref< IStream > dataStream = instance->writeData(dataName);
		if (!dataStream)
			return nullptr;

		if (!StreamCopy(dataStream, stream).execute(dataSize))
			return nullptr;

		dataStream->close();
	}

	if (!instance->commit())
		return nullptr;

	return instance;
}

}
