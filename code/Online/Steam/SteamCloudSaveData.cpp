#include <cstring>
#include <steam/steam_api.h>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/MD5.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Online/Steam/SteamCloudSaveData.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SteamCloudSaveData", SteamCloudSaveData, ISaveDataProvider)

bool SteamCloudSaveData::enumerate(std::set< std::wstring >& outSaveDataIds)
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

bool SteamCloudSaveData::get(const std::wstring& saveDataId, Ref< ISerializable >& outAttachment)
{
	std::string fileName = wstombs(saveDataId);

	int32_t fileSize = SteamRemoteStorage()->GetFileSize(fileName.c_str());
	if (fileSize <= 16)
	{
		log::error << L"Steam save data corrupt; incorrect size" << Endl;
		return false;
	}

	std::vector< uint8_t > buffer(fileSize);
	int32_t nread = SteamRemoteStorage()->FileRead(fileName.c_str(), &buffer[0], fileSize);
	if (nread != fileSize)
	{
		log::error << L"Steam save data corrupt; unable to read data" << Endl;
		return false;
	}

	// Calculate MD5 checksum of actual save data.
	MD5 cs;
	cs.begin();
	cs.feed(&buffer[16], fileSize - 16);
	cs.end();

	// Compare calculated with stored checksums.
	MD5 ss(&buffer[0]);
	if (cs != ss)
	{
		log::error << L"Steam save data corrupt; incorrect checksum " << cs.format() << L" (expected " << ss.format() << L")" << Endl;
		return false;
	}

	// De-serialize actual save data instance.
	DynamicMemoryStream dms(buffer, true, false); dms.seek(IStream::SeekSet, 16);
	outAttachment = BinarySerializer(&dms).readObject();
	return true;
}

bool SteamCloudSaveData::set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace)
{
	std::string fileName = wstombs(saveDataId);

	if (!replace)
	{
		if (SteamRemoteStorage()->FileExists(fileName.c_str()))
			return false;
	}

	DynamicMemoryStream dms(false, true);

	const uint8_t pad[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	dms.write(pad, 16);

	BinarySerializer(&dms).writeObject(attachment);

	std::vector< uint8_t >& buffer = dms.getBuffer();
	T_ASSERT (buffer.size() >= 16);

	MD5 cs;
	cs.begin();
	cs.feed(&buffer[16], buffer.size() - 16);
	cs.end();

	std::memcpy(&buffer[0], cs.get(), 16);

	if (!SteamRemoteStorage()->FileWrite(fileName.c_str(), &buffer[0], buffer.size()))
		return false;

	return true;
}

bool SteamCloudSaveData::remove(const std::wstring& saveDataId)
{
	std::string fileName = wstombs(saveDataId);
	return SteamRemoteStorage()->FileDelete(fileName.c_str());
}

	}
}
