#include "Database/Isolate.h"
#include "Database/Instance.h"
#include "Database/Group.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Serialization/BinarySerializer.h"

namespace traktor
{
	namespace db
	{

bool Isolate::createIsolatedInstance(Instance* instance, Stream* stream)
{
	T_ASSERT (stream && stream->canWrite());

	Writer writer(stream);

	writer << instance->getName();
	writer << instance->getGuid().format();

	std::vector< std::wstring > dataNames;
	writer << uint32_t(instance->getDataNames(dataNames));

	Ref< Serializable > object = instance->getObject();
	if (!BinarySerializer(stream).writeObject(object))
		return false;

	for (std::vector< std::wstring >::iterator i = dataNames.begin(); i != dataNames.end(); ++i)
	{
		writer << *i;

		Ref< Stream > dataStream = instance->readData(*i);
		T_ASSERT (dataStream);

		int32_t dataSize = dataStream->available(); 
		writer << dataSize;

		if (!StreamCopy(stream, dataStream).execute(dataSize))
			return false;
	}

	return true;
}

Instance* Isolate::createInstanceFromIsolation(Group* group, Stream* stream)
{
	T_ASSERT (stream && stream->canRead());

	Reader reader(stream);

	std::wstring instanceName, instanceGuid;
	reader >> instanceName;
	reader >> instanceGuid;

	uint32_t dataNames;
	reader >> dataNames;

	Guid guid(instanceGuid);
	if (!guid.isValid())
		return false;

	Ref< Instance > instance = group->createInstance(instanceName, CifReplaceExisting, &guid);
	if (!instance)
		return 0;

	Ref< Serializable > object = BinarySerializer(stream).readObject();
	if (!instance->setObject(object))
		return 0;

	for (uint32_t i = 0; i < dataNames; ++i)
	{
		std::wstring dataName;
		reader >> dataName;

		int32_t dataSize;
		reader >> dataSize;

		Ref< Stream > dataStream = instance->writeData(dataName);
		if (!dataStream)
			return 0;

		if (!StreamCopy(dataStream, stream).execute(dataSize))
			return 0;

		dataStream->close();
	}

	if (!instance->commit())
		return 0;

	return instance;
}

	}
}
