#include <steam/steam_api.h>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Online/Steam/SteamSaveData.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SteamSaveData", SteamSaveData, ISaveDataProvider)

bool SteamSaveData::enumerate(std::set< std::wstring >& outSaveDataIds)
{
	int32_t fileCount = SteamRemoteStorage()->GetFileCount();
	int32_t fileSize;

	for (int32_t i = 0; i < fileCount; ++i)
	{
		const char* fileName = SteamRemoteStorage()->GetFileNameAndSize(i, &fileSize);
		if (fileName)
			outSaveDataIds.insert(mbstows(fileName));
	}

	return true;
}

bool SteamSaveData::get(std::wstring& saveDataId, Ref< ISerializable >& outAttachment)
{
	std::string fileName = wstombs(saveDataId);

	int32_t fileSize = SteamRemoteStorage()->GetFileSize(fileName.c_str());
	if (fileSize <= 0)
		return false;

	std::vector< uint8_t > buffer(fileSize);
	int32_t nread = SteamRemoteStorage()->FileRead(fileName.c_str(), &buffer[0], fileSize);
	if (nread != fileSize)
		return false;

	DynamicMemoryStream dms(buffer, true, false);
	outAttachment = BinarySerializer(&dms).readObject();

	return true;
}

bool SteamSaveData::set(std::wstring& saveDataId, const ISerializable* attachment, bool replace)
{
	std::string fileName = wstombs(saveDataId);

	if (!replace)
	{
		if (SteamRemoteStorage()->FileExists(fileName.c_str()))
			return false;
	}

	DynamicMemoryStream dms(false, true);
	BinarySerializer(&dms).writeObject(attachment);

	const std::vector< uint8_t >& buffer = dms.getBuffer();
	if (!SteamRemoteStorage()->FileWrite(fileName.c_str(), &buffer[0], buffer.size()))
		return false;

	return true;
}

	}
}
