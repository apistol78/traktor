/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/System/OS.h"
#include "Online/Lan/LanSaveData.h"

namespace traktor::online
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LanSaveData", LanSaveData, ISaveDataProvider)

LanSaveData::LanSaveData()
{
	const std::wstring savePath = OS::getInstance().getWritableFolderPath() + L"/Traktor/Lan/Save";
	FileSystem::getInstance().makeAllDirectories(savePath);
}

bool LanSaveData::enumerate(std::set< std::wstring >& outSaveDataIds)
{
	const std::wstring savePath = OS::getInstance().getWritableFolderPath() + L"/Traktor/Lan/Save";

	RefArray< File > saveFiles;
	FileSystem::getInstance().find(savePath + L"/*.save", saveFiles);
	for (RefArray< File >::const_iterator i = saveFiles.begin(); i != saveFiles.end(); ++i)
		outSaveDataIds.insert((*i)->getPath().getFileNameNoExtension());

	return true;
}

bool LanSaveData::get(const std::wstring& saveDataId, Ref< ISerializable >& outAttachment)
{
	const std::wstring saveFile = OS::getInstance().getWritableFolderPath() + L"/Traktor/Lan/Save/" + saveDataId + L".save";

	Ref< IStream > file = FileSystem::getInstance().open(saveFile, File::FmRead);
	if (!file)
		return false;

	outAttachment = BinarySerializer(file).readObject();
	file->close();

	return true;
}

bool LanSaveData::set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace)
{
	const std::wstring saveFile = OS::getInstance().getWritableFolderPath() + L"/Traktor/Lan/Save/" + saveDataId + L".save";

	if (!replace)
	{
		if (FileSystem::getInstance().exist(saveFile))
			return false;
	}

	Ref< IStream > file = FileSystem::getInstance().open(saveFile, File::FmWrite);
	if (!file)
		return false;

	const bool result = BinarySerializer(file).writeObject(attachment);
	file->close();

	return result;
}

bool LanSaveData::remove(const std::wstring& saveDataId)
{
	const std::wstring saveFile = OS::getInstance().getWritableFolderPath() + L"/Traktor/Lan/Save/" + saveDataId + L".save";
	return FileSystem::getInstance().remove(saveFile);
}

}
