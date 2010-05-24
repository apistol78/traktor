#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Online/Steam/SaveGameSteam.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SaveGameSteam", SaveGameSteam, ISaveGame)

SaveGameSteam::SaveGameSteam(const std::wstring& name)
:	m_name(name)
{
}

std::wstring SaveGameSteam::getName() const
{
	return m_name;
}

Ref< ISerializable > SaveGameSteam::getAttachment() const
{
	std::string fileName = wstombs(m_name);

	int32_t fileSize = SteamRemoteStorage()->GetFileSize(fileName.c_str());
	if (fileSize <= 0)
		return 0;

	std::vector< uint8_t > buffer(fileSize);
	int32_t nread = SteamRemoteStorage()->FileRead(fileName.c_str(), &buffer[0], fileSize);
	if (nread != fileSize)
		return 0;

	DynamicMemoryStream dms(buffer, true, false);
	return BinarySerializer(&dms).readObject();
}

	}
}
