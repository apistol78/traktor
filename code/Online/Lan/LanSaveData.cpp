/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/System/OS.h"
#include "Online/Lan/LanSaveData.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LanSaveData", LanSaveData, ISaveDataProvider)

LanSaveData::LanSaveData()
{
	std::wstring savePath = OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB/Save";
	FileSystem::getInstance().makeAllDirectories(savePath);
}

bool LanSaveData::enumerate(std::set< std::wstring >& outSaveDataIds)
{
	std::wstring savePath = OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB/Save";

	RefArray< File > saveFiles;
	FileSystem::getInstance().find(savePath + L"/*.save", saveFiles);

	for (RefArray< File >::const_iterator i = saveFiles.begin(); i != saveFiles.end(); ++i)
		outSaveDataIds.insert((*i)->getPath().getFileNameNoExtension());

	return true;
}

bool LanSaveData::get(const std::wstring& saveDataId, Ref< ISerializable >& outAttachment)
{
	std::wstring saveFile = OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB/Save/" + saveDataId + L".save";

	Ref< IStream > file = FileSystem::getInstance().open(saveFile, File::FmRead);
	if (!file)
		return false;

	outAttachment = BinarySerializer(file).readObject();
	file->close();

	return true;
}

bool LanSaveData::set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace)
{
	std::wstring saveFile = OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB/Save/" + saveDataId + L".save";

	if (!replace)
	{
		if (FileSystem::getInstance().exist(saveFile))
			return false;
	}

	Ref< IStream > file = FileSystem::getInstance().open(saveFile, File::FmWrite);
	if (!file)
		return false;

	bool result = BinarySerializer(file).writeObject(attachment);
	file->close();

	return result;
}

bool LanSaveData::remove(const std::wstring& saveDataId)
{
	std::wstring saveFile = OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB/Save/" + saveDataId + L".save";
	return FileSystem::getInstance().remove(saveFile);
}

	}
}
