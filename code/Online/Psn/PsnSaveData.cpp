#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Online/Psn/PsnSaveData.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.PsnSaveData", PsnSaveData, ISaveDataProvider)

bool PsnSaveData::enumerate(std::set< std::wstring >& outSaveDataIds)
{
	return true;
}

bool PsnSaveData::get(std::wstring& saveDataId, Ref< ISerializable >& outAttachment)
{
	return false;
}

bool PsnSaveData::set(std::wstring& saveDataId, const ISerializable* attachment, bool replace)
{
	return false;
}

	}
}
